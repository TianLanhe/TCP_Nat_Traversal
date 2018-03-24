#include "../include/natchecker/NatCheckerClient.h"
#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/socket/ServerSocket.h"

using namespace Lib;
using namespace std;

NatCheckerClient::NatCheckerClient(const string& addr, port_type port):m_isConnected(false){
    bind(addr,port);
}

bool NatCheckerClient::bind(const string& addr, port_type port){
    CHECK_OPERATION_EXCEPTION(!isBound());

    ClientSocket *client = ReuseSocketFactory::GetInstance()->GetClientSocket();

    if(client->bind(addr,port)){
        m_sockets.push_back(client);
        return true;
    }else{
        return false;
    }
}

bool NatCheckerClient::isBound() const {
    return !m_sockets.empty();
}

NatCheckerClient::~NatCheckerClient(){
    for(vector<ClientSocket*>::size_type i=0;i<m_sockets.size();++i)
        delete m_sockets[i];
}

std::string NatCheckerClient::getExternAddr() const {
    CHECK_OPERATION_EXCEPTION(isConnected());

    return m_ext_addr;
}

typename NatCheckerClient::port_type NatCheckerClient::getExternPort() const {
    CHECK_OPERATION_EXCEPTION(isConnected());

    return m_ext_port;
}

string NatCheckerClient::getLocalAddr() const {
    CHECK_OPERATION_EXCEPTION(isBound());

    return _getMainSocket()->getAddr();
}

typename NatCheckerClient::port_type NatCheckerClient::getLocalPort() const {
    CHECK_OPERATION_EXCEPTION(isBound());

    return _getMainSocket()->getPort();
}

bool NatCheckerClient::connect(const std::string& addr, port_type port){
    CHECK_OPERATION_EXCEPTION(_getMainSocket());

    ClientSocket *client = _getMainSocket();

	// 连接到 STUN 服务器，与其进行交互获取 NAT 类型
    if(!client->isConnected() && !client->connect(addr,port,1)){
        return false;
    }

    // 若存在 NAT，服务器会探测其 Filter 类型，需要这边在相同地址和端口监听连接（无需其他操作，只为 STUN 服务器连接内网，结束后删除即可）
    ServerSocket *server = ReuseSocketFactory::GetInstance()->GetServerSocket();
    if(!server->bind(client->getAddr(),client->getPort()) || !server->listen(5)){
        delete server;
        return false;
    }

    TransmissionProxy proxy(client);
    TransmissionData data;

	// 向 STUN 服务器发送这边的 IP 和端口，以供其判断是否存在NAT，也可用于之后将内网地址发给对等端以尝试内网直接相连
    data.add(LOCAL_IP,client->getAddr());
    data.add(LOCAL_PORT,client->getPort());
    proxy.write(data);

	// 读取第一次从 STUN 服务器返回的消息，里面包含两种情况：
	// 1. 不存在NAT，探测停止。返回信息包括：不需要继续探测的标志
	// 2. 服务器发现外网 IP 与客户端发送的不一致，判断存在 NAT ，那边已进行了 Filter 探测，接下来需要客户端配合连接以判断 Map 类型以及端口映射规律
    //    返回信息包括：需要继续探测的标志、外网地址、Filter类型、客户端需要进行连接的下一个地址
    data.clear();
    data = proxy.read();
    
    // 读取到 STUN 服务器返回的消息，证明必然完成 Filter 探测，可关闭监听端
    delete server;
    server = NULL;

    if(!data.isMember(CONTINUE))
        return false;

    bool continued = data.getBool(CONTINUE);

    if(!continued){
        m_natType.setNatType(false);
        m_ext_port = client->getPort();
        m_ext_addr = client->getAddr();
        m_isConnected = true;
        // Review: 是否需要关闭 client?
        return true;
    }

    if(!data.isMember(FILTER_TYPE) || !data.isMember(EXTERN_IP) || !data.isMember(EXTERN_PORT))
        return false;

	// 处理返回信息中的 Filter 类型、外网地址
    filter_type filterType = (filter_type)(data.getInt(FILTER_TYPE));
    map_type mapType;

    m_ext_addr = data.getString(EXTERN_IP);
    m_ext_port = data.getInt(EXTERN_PORT);

	// 接下来读取待连接地址，进行连接，然后读取服务器返回信息，判断是否需要继续连接，循环反复，直到
	// STUN 服务器表示不用继续了，已经探测出 Map 类型以及端口递增规律了，则发送探测标志为假的返回信息通知客户端停止
	// 若需要继续连接，返回信息包括：需要继续探测的标志、需要进行连接的下一个地址
	// 若停止连接，完成探测，返回信息包括：不需要继续探测的标志、NAT 的 Map 类型，但端口递增规律对客户端透明，只有 STUN 服务器知道
    while(continued){
        if(!data.isMember(CHANGE_IP) || !data.isMember(CHANGE_PORT))
            return false;

        string changeIp = data.getString(CHANGE_IP);
        port_type changePort = data.getInt(CHANGE_PORT);

        ClientSocket *c = ReuseSocketFactory::GetInstance()->GetClientSocket();
        if(!c->bind(client->getAddr(),client->getPort()) || !c->connect(changeIp,changePort,1)){
            return false;
        }else{
            m_sockets.push_back(c);
        }

        proxy.setSocket(c);

        data.clear();
        data = proxy.read();

        if(!data.isMember(CONTINUE))
            return false;

        continued = data.getBool(CONTINUE);
    }

    if(!data.isMember(MAP_TYPE))
        return false;

    mapType = (map_type)(data.getInt(MAP_TYPE));

    m_natType.setNatType(true,mapType,filterType);
    m_isConnected = true;
    // Review: 如何处理那些连接的socket
    return true;
}

