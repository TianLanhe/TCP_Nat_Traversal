#include "natchecker/MultNatCheckerServerSlave.h"
#include "include/socket/ReuseSocketFactory.h"
#include "include/socket/ServerSocket.h"
#include "include/socket/ClientSocket.h"
#include "include/transmission/TransmissionData.h"
#include "include/transmission/TransmissionProxy.h"
#include "include/Log.h"
#include "include/NatType.h"

#include <string>

using namespace Lib;
using namespace std;

MultNatCheckerServerSlave::MultNatCheckerServerSlave(port_type port):m_port(port)
{
    m_server =  ReuseSocketFactory::GetInstance()->GetServerSocket();
}

MultNatCheckerServerSlave::~MultNatCheckerServerSlave()
{
    if(m_server)
        delete m_server;
}

bool MultNatCheckerServerSlave::setListenNum(size_t num){
    if(!m_server)
        return false;

    if(!m_server->isBound() && !m_server->bind(m_port))
        return false;

    return !m_server->isListen() && m_server->listen(num);
}

void MultNatCheckerServerSlave::handle_request(ClientSocket* client){
    Log(INFO) << "master checker server has connected: " << client->getPeerAddr() << ":" << client->getPeerPort() << eol;

    TransmissionProxy proxy(client);
    TransmissionData data = proxy.read();

    if(!data.isMember(CHECK_TYPE)){
        Log(ERROR) << "data have no member \"CHECK_TYPE\"" << eol;
        return;
    }

    string checkType = data.getString(CHECK_TYPE);
    Log(INFO) << "check type: " << checkType << eol;

    // 读取 CHECK_TPYE 判断操作类型，如果 master 要求辅助探测 filter 类型，则读取出要绑定的端口，以及要连接的地址和端口，然后尝试连接，并把连接结果返回给 master
    // 如果 master 要求辅助探测 map 类型，则读取出要绑定的端口，然后监听该端口等待 NAT 连接，接收到连接后，读取出对方的 IP 和端口返回给 master，并断开连接
    if(checkType == CHECK_MAPPING){
        // PRE_IP 和 PRE_PORT 用于 slave 判断其 Mapping 类型，然后由 slave 返回给 NAT 其 Mapping 类型
        if(!data.isMember(BIND_PORT) || !data.isMember(PRE_IP) || !data.isMember(PRE_PORT)){
            Log(ERROR) << "data have no member \"BIND_PORT\" \"PRE_IP\" \"PRE_PORT\" : " << data.isMember(BIND_PORT) << " " << data.isMember(PRE_IP) << " " << data.isMember(PRE_PORT) << eol;
            return;
        }

        port_type bind_port = data.getInt(BIND_PORT);
        ip_type pre_ip = data.getString(PRE_IP);
        port_type pre_port = data.getInt(PRE_PORT);

        ServerSocket *s = ReuseSocketFactory::GetInstance()->GetServerSocket();
        if(!s){
            Log(ERROR) << "create server socket error" << eol;
            return;
        }

        if(!s->bind(bind_port) || !s->listen(DEFAULT_LISTEN_NUM)){
            Log(ERROR) << "server socket bind(" << bind_port << ") or listen error" << eol;
            delete s;
            return;
        }

        // 等待 NAT 连接
        ClientSocket *c = s->accept(DEFAULT_ACCEPT_TIMEOUT);
        if(c == NULL){
            Log(ERROR) << "server socket accept error" << eol;
            delete s;
            return;
        }

        ip_type ext_ip = c->getPeerAddr();
        port_type ext_port = c->getPeerPort();

        Log(INFO) << "client previous addr: " << pre_ip << ":" << pre_port << ", extern addr: " << ext_ip << ":" << ext_port << eol;

        nat_type::map_type mapType;

        // 根据观察到的外网地址判断 NAT 的 Mapping 类型
        if(pre_ip != ext_ip){// 改变目的IP，NAT 分配给同一个内网主机的 IP 却变了
            Log(WARN) << "The NAT allocate the different global IP to the same host, pre_addr: " << pre_ip << ":" << pre_port << ", ext_addr: " << ext_ip << ":" << ext_port << eol;
            mapType = nat_type::UNKNOWN;
        }else if(pre_port == ext_port){ // 连接到 master 的端口和连接到 slave 的端口相同，表示是 EI 的 Mapping 规则
            mapType = nat_type::ENDPOINT_INDEPENDENT;
        }else{ // 连接到 master 的端口和连接到 slave 的端口不同，表示是 AD 的 Mapping 规则， master 前面已经测试了不是 PD 了，亦即需要 slave 辅助探测时只可能是 EI 或者 AD
            mapType = nat_type::ADDRESS_DEPENDENT;
        }

        // 将外网地址返回给 master
        data.clear();
        data.add(RESPONSE_IP,ext_ip);
        data.add(RESPONSE_PORT,ext_port);

        if(!proxy.write(data)){
            Log(ERROR) << "write data to master error" << eol;
            delete c;
            delete s;
            return;
        }

        // 向 NAT 返回 Mapping 类型信息和结束探测的信息
        data.clear();
        data.add(MAP_TYPE,mapType);
        data.add(CONTINUE,false);

        proxy.setSocket(c);
        if(!proxy.write(data)){
            Log(ERROR) << "write data to nat client error" << eol;
            delete c;
            delete s;
            return;
        }

        delete c;
        delete s;
    }else if(checkType == CHECK_FILTERING){
        if(!data.isMember(BIND_PORT) || !data.isMember(CONNECT_IP) || !data.isMember(CONNECT_PORT)){
            Log(ERROR) << "data have no member \"BIND_PORT\" \"CONNECT_IP\" \"CONNECT_PORT\" : " << data.isMember(BIND_PORT) << " " << data.isMember(CONNECT_IP) << " " << data.isMember(CONNECT_PORT) << eol;
            return;
        }

        port_type bind_port = data.getInt(BIND_PORT);
        ip_type connect_ip = data.getString(CONNECT_IP);
        port_type connect_port = data.getInt(CONNECT_PORT);

        ClientSocket *c = ReuseSocketFactory::GetInstance()->GetClientSocket();
        if(!c){
            Log(ERROR) << "create client socket error" << eol;
            return;
        }

        if(!c->bind(bind_port)){
            Log(ERROR) << "client socket bind " << bind_port << " error" << eol;
            delete c;
            return;
        }

        bool canConnect = c->connect(connect_ip,connect_port);

        Log(INFO) << "slave(" << c->getAddr() << ":" << c->getPort() << ") connect to client(" << connect_ip << ":" << connect_port << ")";
        if(canConnect)
            Log(INFO) << " successfully!" << eol;
        else
            Log(INFO) << " failed!" << eol;

        delete c;
        c = NULL;

        data.clear();
        data.add(CAN_CONNECT,canConnect);
        if(!proxy.write(data)){
            Log(ERROR) << "write data to master error " << eol;
            return;
        }
    }else{
        Log(ERROR) << "error check type: " << checkType << eol;
    }
}

void MultNatCheckerServerSlave::waitForClient(){
    if(!m_server->isListen() && !setListenNum(DEFAULT_LISTEN_NUM))
        THROW_EXCEPTION(InvalidOperationException,"bind or listen error");

    ClientSocket *client;
    while(client = m_server->accept()){
        handle_request(client);			// Review：目前的处理是不要并发执行，来一个处理一个，因为检测 NAT 类型的过程中会等待对方发起连接，如果并发会混，也许这个线程接受到的 socket 连接是另一个线程需要的
        delete client;                  // 后续为每次请求分配一个 unique id 进行区分
        client = NULL;
    }
}
