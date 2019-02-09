#include "include/nattraversal/NatTraversalClient.h"
#include "include/socket/ClientSocket.h"
#include "include/socket/DefaultSocketFactory.h"
#include "include/transmission/TransmissionData.h"
#include "include/transmission/TransmissionProxy.h"
#include "include/natchecker/NatCheckerClient.h"
#include "include/traversalcommand/TraversalCommand.h"
#include "NatTraversalCommon.h"
#include "include/Utility.h"
#include "include/socket/SocketSelector.h"
#include "include/socket/SocketSelectorFactory.h"
#include "include/Exception.h"

using namespace std;
using namespace Lib;

NatTraversalClient::NatTraversalClient(const std::string& identifier):m_identifier(identifier),
    m_isReady(false),m_isConnecting(false){
    m_socket = DefaultSocketFactory::GetInstance()->GetClientSocket();
}

NatTraversalClient::~NatTraversalClient(){
    if(m_socket)
        delete m_socket;
}

bool NatTraversalClient::enroll(const ip_type& ip,port_type port){
    if(!m_socket->connect(ip,port))
        return false;

    TransmissionProxy proxy(m_socket);
    TransmissionData data;

    data.add(IDENTIFIER,m_identifier);

    return proxy.write(data);
}

bool NatTraversalClient::hasEnrolled(){
    return m_socket->isConnected();
}

bool NatTraversalClient::isReadyToAccept(){
    bool ret;
    m_mutex.lock();
    ret = m_isReady;
    m_mutex.unlock();
    return ret;
}

bool NatTraversalClient::_setReadySafely(bool ready){
    bool ret;
    m_mutex.lock();
    ret = m_isReady;
    m_isReady = ready;
    m_mutex.unlock();
    return ret;
}

bool NatTraversalClient::setReadyToAccept(bool ready){
    if(isReadyToAccept() == ready)
        return true;

    CHECK_OPERATION_EXCEPTION(hasEnrolled());

    TransmissionProxy proxy(m_socket);
    TransmissionData data;

    data.add(READY_TO_CONNECT,ready);	// 向服务器通知本地端的设置
    if(!proxy.write(data))
        return false;

    _setReadySafely(ready);

    return true;
}

ClientSocket::port_type NatTraversalClient::_genClientPort(){
    return Lib::Util::getRandomNumByRange(1024,65536);
}

ClientSocket* NatTraversalClient::_checkNatTypeAndConnect(const ip_type& stun_ip, port_type stun_port){
    ClientSocket *ret = NULL;

    // 18/09/18:每次生成新的端口进行连接，灵活性更大，也便于单机调试
    port_type port = _genClientPort();

    // 进行 NAT 类型检测
    NatCheckerClient client(m_identifier,m_socket->getAddr(),port);
    if(!client.connect(stun_ip,stun_port))
        return ret;

    // 等待 TYPE 回复
    TransmissionProxy proxy(m_socket);
    TransmissionData data = proxy.read();

    if(!data.isMember(TYPE))
        return ret;

    // 根据穿越的 TYPE 取得对应的具体穿越操作
    TraversalCommand *command = GetTraversalCommandByType((TraversalCommand::TraversalType)(data.getInt(TYPE)));
    if(command == NULL)
        return ret;

	// 进行穿越操作，返回连接的 socket
    ret = command->traverse(data,m_socket->getAddr(),port);

    delete command;

    return ret;
}

ClientSocket* NatTraversalClient::connectToPeerHost(const std::string& identifier){
    CHECK_OPERATION_EXCEPTION(hasEnrolled());

	// NAT 穿越客户端流程：首先 Client 与服务器建立持久 TCP 连接，以保持通信与传递信令
	// 客户端有两方面需要做的，一方面随时监听服务器端发来的 Other Client 发起的连接请求，这个在下面一个函数处理
	// 另一方面，客户端需要能主动发起与对等的连接，下面是主动发起连接的处理流程：
	// 首先客户端设置 m_isReady 标志变量为 false（要求线程安全），以暂停监听被动连接函数的处理
	// 接着 Client 向服务器端发送连接请求，包含对等端的标识符，以表示是要跟谁连接。
	// Client 根据收到的回复进行处理，若服务器返回可以连接，则进行 NAT 类型检测，服务器再根据双方的 NAT 类型通知 Client 要怎么做；
    // 若服务器拒绝连接（对等端没有登录或还没准备好接受连接或该客户端没有登录），则返回失败的标志，Client 直接返回
	// NAT 类型检测成功后，Client 等待服务器端的连接执行，是等待监听还是打洞监听还是直接连接还是扫描连接

    ClientSocket *ret = NULL;
    
    m_mtx.lock();
    m_isConnecting = true;
    m_mtx.unlock();

    bool isReady = _setReadySafely(false);

    TransmissionProxy proxy(m_socket);
    TransmissionData data;

	//	发 PEER_HOST, 等待 CAN_CONNECT 、（ STUN_IP 、STUN_PORT ）回复
    data.add(PEER_HOST,identifier);
    if(!proxy.write(data))		// 发送对等方标识符，表示要与对方进行 P2P 连接，需要服务器协助
        goto r;
    
    data.clear();
    data = proxy.read();
    
    if(!data.isMember(CAN_CONNECT) || !data.getBool(CAN_CONNECT))	// 服务器返回拒绝连接
    	goto r;
    	
    if(!data.isMember(STUN_IP) || !data.isMember(STUN_PORT))	// 如果可以连接，会携带着 NAT 类型检测的服务器地址，Client 往这个地址去连接
    	goto r;
    	
    ret = _checkNatTypeAndConnect(data.getString(STUN_IP),data.getInt(STUN_PORT));

r:
    {
        unique_lock<mutex> lck(m_mutex);
        if(isReady){
            m_isReady = true;
            m_conVar.notify_one();
        }
    }

    {
        unique_lock<mutex> ulck(m_mtx);
        m_isConnecting = false;
        m_cnv.notify_one();
    }

    return ret;
}

ClientSocket* NatTraversalClient::waitForPeerHost(){
    CHECK_OPERATION_EXCEPTION(hasEnrolled());

    {
        unique_lock<mutex> lck(m_mtx);
        while(m_isConnecting)
            m_cnv.wait(lck);

        if(!setReadyToAccept())
            return NULL;
    }

	SocketSelector *selector = SocketSelectorFactory::GetInstance()->GetReadSelector();
	selector->add(m_socket);
	while (1) {
		SocketSelector::SocketVector vecSockets = selector->select();
		CHECK_STATE_EXCEPTION(vecSockets.size() == 1 && vecSockets[0] == m_socket);

		if (isReadyToAccept()) {
			TransmissionProxy proxy(m_socket);
			TransmissionData data = proxy.read();

			delete selector;

			if (!data.isMember(STUN_IP) || !data.isMember(STUN_PORT))
				return NULL;

			return _checkNatTypeAndConnect(data.getString(STUN_IP), data.getInt(STUN_PORT));
		}
		else {
			unique_lock<mutex> lck(m_mutex);
			while (!m_isReady)
				m_conVar.wait(lck);
		}
	}

    /*DefaultClientSocket *defaultClientSocket = dynamic_cast<DefaultClientSocket*>(m_socket);
    if(defaultClientSocket == NULL)
    	return NULL;

    int fd = defaultClientSocket->_getfd();
    fd_set set;

    while(1){
        FD_ZERO(&set);
        FD_SET(fd,&set);

        int ret = select(fd + 1,&set,NULL,NULL,NULL);
        if(ret == -1){
            THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalClient::waitForPeerHost");
        }else if(ret == 0){
            THROW_EXCEPTION(ErrorStateException,"select timeout in NatTraversalClient::waitForPeerHost");
        }else{
            CHECK_STATE_EXCEPTION(ret == 1);

            if(isReadyToAccept()){
                TransmissionProxy proxy(m_socket);
                TransmissionData data = proxy.read();

                if(!data.isMember(STUN_IP) || !data.isMember(STUN_PORT))
                    return NULL;

                return _checkNatTypeAndConnect(data.getString(STUN_IP),data.getInt(STUN_PORT));
            }else{
                unique_lock<mutex> lck(m_mutex);
                while(!m_isReady)
                    m_conVar.wait(lck);
            }
        }
    }*/
}
