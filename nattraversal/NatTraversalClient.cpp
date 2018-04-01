#include "../include/nattraversal/NatTraversalClient.h"
#include "../include/socket/ClientSocket.h"
#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../include/natchecker/NatCheckerClient.h"
#include "../socket/DefaultClientSocket.h"
#include "NatTraversalCommon.h"

#include <sys/select.h>

using namespace std;
using namespace Lib;

NatTraversalClient::NatTraversalClient(const std::string& identifier):m_identifier(identifier),
    m_isReady(false),m_isConnecting(false){
    m_socket = new DefaultClientSocket();
}

NatTraversalClient::~NatTraversalClient(){
    if(m_socket)
        delete m_socket;
}

bool NatTraversalClient::enroll(const ip_type& ip,port_type port){
    if(!m_socket->connect(ip,port,1))
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

void NatTraversalClient::setReadySafely(bool ready){
    m_mutex.lock();
    m_isReady = ready;
    m_mutex.unlock();
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

    setReadySafely(ready);

    return true;
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
    ip_type stun_ip;
    port_type stun_port;

    NatCheckerClient client(m_identifier,m_socket->getAddr(),CLIENT_DEFAULT_PORT);
    
    m_mtx.lock();
    m_isConnecting = true;
    m_mtx.unlock();

    _setReadySafely(false);

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
    	
    if(!data.isMember(STUN_IP) || !data.isMember(STUN_PORT))
    	goto r;
    	
    stun_ip = data.getString(STUN_IP);		// 如果可以连接，会携带着 NAT 类型检测的服务器地址，Client 往这个地址去连接
    stun_port = data.getInt(STUN_PORT);
    	
    // 进行 NAT 类型检测
    if(!client.connect(stun_ip,stun_port))
    	goto r;
    	
    // 等待 COMMAND 回复
    data.clear();
    data = proxy.read();

    if(!data.isMember(COMMAND))
        goto r;

    // TODO 进行穿越连接操作
    //ret = OperationFactory::GetInstance()->StartOperation(data.getString(COMMAND),data);

r:
    unique_lock<mutex> lck(m_mutex);
    m_isReady = true;
    m_conVar.notify_one();

    unique_lock<mutex> ulck(m_mtx);
    m_isConnecting = false;
    m_conVar.notify_one();

    return ret;
}

ClientSocket* NatTraversalClient::waitForPeerHost(){
    CHECK_OPERATION_EXCEPTION(hasEnrolled());

    {
        unique_lock lck(m_mtx);
        while(m_isConnecting)
            m_cnv.wait(lck);
    }

    if(!isReadyToAccept() && !setReadyToAccept())
        return NULL;

    DefaultClientSocket *defaultClientSocket = dynamic_cast<DefaultClientSocket*>(m_socket);
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
                ClientSocket *ret = NULL;

                TransmissionProxy proxy(m_socket);
                TransmissionData data = proxy.read();

                if(!data.isMember(STUN_IP) || !data.isMember(STUN_PORT))
                    return ret;

                ip_type stun_ip = data.getString(STUN_IP);
                port_type stun_port = data.getInt(STUN_PORT);

                NatCheckerClient client(m_identifier,m_socket->getAddr(),CLIENT_DEFAULT_PORT);

                if(!client.connect(stun_ip,stun_port))
                    return ret;

                data.clear();
                data = proxy.read();

                if(!data.isMember(COMMAND))
                    return ret;

                // TODO
                //ret = OperationFactory::GetInstance()->StartOperation(data.getString(COMMAND),data);

                return ret;
            }else{
                unique_lock<mutex> lck(m_mutex);
                while(!m_isReady)
                    m_conVar.wait(lck);
            }
        }
    }
}
