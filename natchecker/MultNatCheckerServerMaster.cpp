#include "natchecker/MultNatCheckerServerMaster.h"
#include "natchecker/MultNatCheckerServerCommon.h"
#include "include/socket/ReuseSocketFactory.h"
#include "include/socket/ServerSocket.h"
#include "include/socket/ClientSocket.h"
#include "socket/DefaultClientSocket.h"
#include "socket/DefaultServerSocket.h"
#include "include/transmission/TransmissionData.h"
#include "include/transmission/TransmissionProxy.h"
#include "include/database/DataBase.h"
#include "include/Log.h"
#include "include/Utility.h"

#include <sys/select.h>
#include <errno.h>

#include <thread>

extern int errno;

using namespace Lib;
using namespace std;

typedef nat_type::filter_type filter_type;
typedef nat_type::map_type map_type;

MultNatCheckerServerMaster::MultNatCheckerServerMaster(port_type main_port,port_type another_port):m_main_port(main_port),m_another_port(another_port){
    m_server =  ReuseSocketFactory::GetInstance()->GetServerSocket();
}

MultNatCheckerServerMaster::~MultNatCheckerServerMaster(){
    if(m_server)
        delete m_server;

    for(vector<DefaultClientSocket>::size_type i=0;i<m_clientVec.size();++i)
        delete m_clientVec[i];
}

bool MultNatCheckerServerMaster::setListenNum(size_t num){
    if(!m_server)
        return false;

    if(!m_server->isBound() || !m_server->bind(m_main_port))
        return false;

    return !m_server->isListen() && m_server->listen(num);
}

void MultNatCheckerServerMaster::handle_request(ClientSocket* client){
    TransmissionProxy proxy(client);
    TransmissionData data = proxy.read();

    // 先取出 Client 发送过来的本机地址，对比这边观察到的外部地址，如果外部地址与 Client 发送过来的它那边观察到的地址相同，
    // 表示 Client 处于公网中，可以停止 NAT 类型检测。如果不相同，则先进行 NAT 的 Filter 类型检测(Client那边已经开启了监听，等待连接)，
    // Filter 具体检测过程见下。检测完成后，接着要 Client 配合探测 NAT 的 MAP 类型以及端口映射规律，即不断地让 Client 连接服务器要它连接的端口
    // 服务器检查它连接过来的外部地址，若不明确类型及规律，继续让 Client 连接新端口，直到 STUN 服务器探测出需要的了，就发送停止探测的信息
    // 服务器可将整个过程获得到的信息存储起来以便其他用途

    if(!data.isMember(LOCAL_IP) || !data.isMember(LOCAL_PORT) || !data.isMember(IDENTIFIER))
        return;

    ip_type local_ip = data.getString(LOCAL_IP);
    port_type local_port = data.getInt(LOCAL_PORT);

    ip_type ext_ip = client->getPeerAddr();
    port_type ext_port = client->getPeerPort();

    DataRecord record;
    record.setIdentifier(data.getString(IDENTIFIER));

    Address addr(local_ip,local_port);
    record.setLocalAddress(addr);

    addr.ip = ext_ip;
    addr.port = ext_port;
    record.setExtAddress(addr);

    Log(INFO) << "local_ip: " << local_ip << eol;
    Log(INFO) << "local_port: " << local_port << eol;

    Log(INFO) << "ext_ip: " << ext_ip << eol;
    Log(INFO) << "ext_port: " << ext_port << eol;

    // 这是很奇怪的现象，IP 相同端口却改变了
    if(local_ip == ext_ip && local_port != ext_port){
        Log(WARN) << "local IP is equal to extern IP but local port is different from extern port." << eol;
    }

    if(local_ip == ext_ip && local_port == ext_port)	// 内外地址相同，处于公网中，停止检测
    {
        data.clear();
        data.add(CONTINUE,false);
        proxy.write(data);

        record.setNatType(nat_type(false));
    }
    else
    {   // 内外地址不同，存在 NAT，进行 NAT 的 Filter 类型检测 ： 先改变端口进行连接，若不能连接上，则证明是 Address And Port Dependent，不需要 Slave 辅助检测
        // 若改变端口能连接上，则说明可能是 EndPoint Independent 或 Address Dependent 的 Filter 规则，需要 Slave 辅助测试，若 Slave 能连接上，则表明是 EndPoint Independent
        // 否则是 Address Dependent
        ClientSocket *c = ReuseSocketFactory::GetInstance()->GetClientSocket();
        if(!c->bind(m_server->getAddr(),m_another_port)){
            delete c;
            return;
        }

        bool canConnect = c->connect(ext_ip,ext_port,_getConnectRetryTime());
        filter_type filterType;
        if(!canConnect){
            Log(INFO) << m_server->getAddr() << ":" << m_another_port << " fail to connect to " << ext_ip << ":" << ext_port << eol;
            Log(INFO) << "filterType: ADDRESS_AND_PORT_DEPENDENT" << eol;
            filterType = nat_type::ADDRESS_AND_PORT_DEPENDENT;
        }else{
            // 改变端口能连接上，表示可能是 EndPoint Independent 或 Address Dependent 的 Filter 规则，需要 Slave 辅助测试
            if(!c->reopen() || !c->connect(m_slaveIp,SLAVE_LISTEN_PORT)){
                delete c;
                return;
            }

            // 连接到 Slave ，发送指令让其连接
            data.clear();
            data.add(CHECK_TYPE,CHECK_FILTERING);
            data.add(BIND_PORT,m_main_port);
            data.add(CONNECT_IP,ext_ip);
            data.add(CONNECT_PORT,ext_port);

            TransmissionProxy slaveProxy(c);
            if(!slaveProxy.write(data)){
                delete c;
                return;
            }

            data = slaveProxy.read();
            if(!data.isMember(CAN_CONNECT)){
                delete c;
                return;
            }

            // Slave 返回测试结果，能连接表示换一个 IP 能连接，是 EndPoint Independent 类型，若连接不上则说明是 Address Dependent
            canConnect = data.getBool(CAN_CONNECT);
            if(canConnect){
                Log(INFO) << m_slaveIp << ":" << m_main_port << " connect to " << ext_ip << ":" << ext_port << " successfully" << eol;
                filterType = nat_type::ENDPOINT_INDEPENDENT;
                Log(INFO) << "filterType: ENDPOINT_INDEPENDENT" << eol;
            }else{
                Log(INFO) << m_slaveIp << ":" << m_main_port << " fail to connect to " << ext_ip << ":" << ext_port << eol;
                filterType = nat_type::ADDRESS_DEPENDENT;
                Log(INFO) << "filterType: ADDRESS_DEPENDENT" << eol;
            }
        }

        delete c;
        c = NULL;

        // 首先服务器在 IP1:Prot2 监听，通知 Client 连接，若这一次(第2次)所观察到的地址与上一次(第1次)不同，则表示 NAT 属于 Address And Port Dependent，这时候多次通知 Client 进行连接求预测增量
        // 若第2次与第1次观察到的地址相同，则表示 NAT 属于 EndPoint Independent 或者 EndPoint Independent 的 Map 规则，需要 Slave 辅助测试，
        // 则认为 NAT 的 Map 类型为 AD，此时设置其端口为 Extern_Port - 1，预测增量为1

        ServerSocket *s = ReuseSocketFactory::GetInstance()->GetServerSocket();
        if(!(s->bind(m_server->getAddr(),m_another_port) &&
             s->listen(DEFAULT_LISTEN_NUM))){
            delete s;
            return;
        }

        data.clear();
        data.add(CONTINUE,true);
        data.add(FILTER_TYPE,filterType);
        data.add(EXTERN_IP,ext_ip);
        data.add(EXTERN_PORT,ext_port);
        data.add(CHANGE_IP,m_server->getAddr());
        data.add(CHANGE_PORT,m_another_port);

        proxy.write(data);		// STUN 服务器将信息发送给 Client，并等待 Client 连接

        c = s->accept();

        ip_type ext_ip2 = c->getPeerAddr();
        port_type ext_port2 = c->getPeerPort();

        Log(INFO) << "ext_ip2: " << ext_ip2 << eol;
        Log(INFO) << "ext_port2: " << ext_port2 << eol;

        // 假设 NAT 只有一个对外 IP 或 同一个内网主机向外通信时肯定会转换到同一个 IP (也许会改变端口)
        if(ext_ip2 != ext_ip)
            Log(WARN) << "The NAT allocate the different global IP to the same host" << eol;

        if(ext_ip2 == ext_ip && ext_port2 == ext_port) // 第2次的外网地址与第1次的相同
        {
            // 需要借助 Slave 进一步区分 NAT 是 EI 还是 AD 的 Mapping 规则
            ClientSocket *slave = ReuseSocketFactory::GetInstance()->GetClientSocket();
            if(!slave->connect(m_slaveIp,SLAVE_LISTEN_PORT)){
                delete c;
                delete slave;
                return;
            }

            // 向 Slave 写入检查 Mapping 的指令，Slave 会等待 NAT 连接然后把观察到的外网地址返回给 Master
            data.clear();
            data.add(CHECK_TYPE,CHECK_MAPPING);
            data.add(BIND_PORT,m_another_port);
            data.add(PRE_IP,ext_ip2);
            data.add(PRE_PORT,ext_port2);

            proxy.setSocket(slave);
            proxy.write(data);

            // Master 向 Slave 发送指令后，让 NAT 往 Slave进行连接，然后等待 Slave 的结果
            data.clear();
            data.add(CONTINUE,true);
            data.add(CHANGE_IP,m_slaveIp);
            data.add(CHANGE_PORT,m_another_port);

            proxy.setSocket(c);
            proxy.write(data);

            delete c;
            c = NULL;

            // Master 从 Slave 读取结果
            proxy.setSocket(slave);
            data = proxy.read();

            if(!data.isMember(RESPONSE_IP) || !data.isMember(RESPONSE_PORT)){
                delete slave;
                return;
            }

            ip_type ext_ip3 = data.getString(RESPONSE_IP);
            port_type ext_port3 = data.getInt(RESPONSE_PORT);

            delete slave;
            slave = NULL;

            if(ext_ip3 != ext_ip2)
                Log(WARN) << "The NAT allocate the different global IP to the same host" << eol;

            // NAT 连接到 IP1:Port2 的外网地址和连接到 IP2:Port2 的外网地址相同，是 EI 的 Mapping 规则
            if(ext_ip3 == ext_ip2 && ext_port3 == ext_port2){
                // 端口增量为0
                nat_type natType(true,nat_type::ENDPOINT_INDEPENDENT,filterType);
                natType.setPrediction();
                record.setNatType(natType);

                Log(INFO) << "mapType: ENDPOINT_INDEPENDENT" << eol;
            }else{ // 否则是 AD 的 Mapping 规则
                // 端口增量为 ext_port3 - ext_port2
                nat_type natType(true,nat_type::ADDRESS_DEPENDENT,filterType);
                natType.setPrediction(true,ext_port3 - ext_port2);
                record.setNatType(natType);

                // 记得更新 DataRecord 的外部地址，以最新的为准
                record.setExtAddress(Address(ext_ip3,ext_port3 - ext_port2 + ext_port3));

                Log(INFO) << "mapType: ADDRESS_DEPENDENT" << eol;
            }
        }
        else	// 第2次的外网地址与第1次的不同，则是 Address And Port Dependent，进行多次连接预测增量
        {
            int delta_pre = 0;
            int delta_cur = ext_port2 - ext_port;
            size_t try_time = 0;
            port_type ext_port_pre = ext_port2;

            ip_type ext_ip_n;
            port_type ext_port_n;

            ip_type m_main_addr = m_server->getAddr();
            while( (delta_pre != delta_cur || delta_cur + ext_port_pre >= 65535) && try_time++ < _getMaxTryTime()){
                data.clear();
                data.add(CONTINUE,true);
                data.add(CHANGE_IP,m_main_addr);
                data.add(CHANGE_PORT,m_another_port + (port_type)(try_time));

                if(!(s->close() && s->open() &&
                        s->bind(m_main_addr,m_another_port + try_time) &&
                        s->listen(DEFAULT_LISTEN_NUM)))
                    return;

                proxy.setSocket(c);
                proxy.write(data);

                if(c != NULL)
                    delete c;
                c = s->accept();

                ext_ip_n = c->getPeerAddr();
                ext_port_n = c->getPeerPort();

                Log(INFO) << "ext_ip_n: " << ext_ip_n << eol;
                Log(INFO) << "ext_port_n: " << ext_port_n << eol;

                delta_pre = delta_cur;
                delta_cur = ext_port_n - ext_port_pre;
                ext_port_pre = ext_port_n;
            }

            data.clear();
            data.add(MAP_TYPE,nat_type::ADDRESS_AND_PORT_DEPENDENT);
            data.add(CONTINUE,false);

            proxy.setSocket(c);
            proxy.write(data);

            if(c != NULL)
                delete c;
            c = NULL;

            nat_type natType(true,nat_type::ADDRESS_AND_PORT_DEPENDENT,filterType);
            Log(INFO) << "mapType: ADDRESS_AND_PORT_DEPENDENT" << eol;
            if( try_time == _getMaxTryTime() ){ // 端口随机变化
                natType.setPrediction(false);
            }else{  // 设置增量为 delta_cur;
                natType.setPrediction(true,delta_cur);
            }
            record.setNatType(natType);
            record.setExtAddress(Address(ext_ip_n,ext_port_n + delta_cur));	// 记得更新 DataRecord 的外部地址，以最新的为准
        }
        delete s;
        s = NULL;
    }
    //delete client;		// 注意这里不能把 STUN 服务器与 client 发过来的第一次连接给断掉，因为有些对称型 NAT 第一次连接是用内网的源端口
    //client = NULL;			，第二次开始才从一个随机值进行递增分配，如果把这第一次连接断掉了，NAT 可能会复用这个端口，端口猜测会无效

    CHECK_STATE_EXCEPTION(m_database->addRecord(record));
}

void MultNatCheckerServerMaster::waitForClient(){
    if(!m_server->isListen() && !setListenNum(DEFAULT_LISTEN_NUM))
        THROW_EXCEPTION(InvalidOperationException,"bind or listen error");

    /*ClientSocket *client;
    while(client = m_main_server->accept()){
        handle_request(client);			// 目前的处理是不要并发执行，来一个处理一个，因为检测 NAT 类型的过程中会等待对方发起连接，如果并发会混，也许这个线程接受到的 socket 连接是另一个线程需要的
    }*/
    // 由于不能立即删了第一次连接的 client，故这里采用 I/O 多路转接的方式，能够监听到 client 断开连接的情况，由客户端主动断开

    DefaultServerSocket *m_socket = dynamic_cast<DefaultServerSocket*>(m_server);
    CHECK_STATE_EXCEPTION(m_socket);

    fd_set set;
    int ret,maxfd,fd;
    while(1)
    {
        FD_ZERO(&set);

        FD_SET(m_socket->_getfd(),&set);		// 把服务器 Socket 和所有接受到的客户端连接 Socket 都添加到 set 中
        maxfd = m_socket->_getfd();

        for(vector<DefaultClientSocket*>::size_type i=0;i<m_clientVec.size();++i){
            fd = m_clientVec[i]->_getfd();

            FD_SET(fd,&set);

            if(maxfd < fd)
                maxfd = fd;
        }

        ret = select(maxfd+1,&set,NULL,NULL,NULL);

        if(ret == -1)
        {
            if(errno == EINTR)
                Log(WARN) << "NatCheckerServer is interrupted at function select" << eol;
            else
                THROW_EXCEPTION(ErrorStateException,"select error in NatCheckerServer::waitForClient");
        }else if(ret == 0)
        {
            THROW_EXCEPTION(ErrorStateException,"select timeout in NatTraversalServer::waitForClient");
        }else
        {
            // 如果服务器端可读，亦即有新客户端请求连接，则接收连接，为其服务，进行 NAT 类型检测
            if(FD_ISSET(m_socket->_getfd(),&set))
            {
                ClientSocket *client = m_socket->accept();

                Log(INFO) << "Client \"" << client->getPeerAddr() << ':' << client->getPeerPort() << "\" connect" << eol;

                DefaultClientSocket *defaultSocket = dynamic_cast<DefaultClientSocket*>(client);
                CHECK_STATE_EXCEPTION(defaultSocket);

                m_clientVec.push_back(defaultSocket);

                handle_request(client);

                --ret;
            }

            // 遍历存储的客户端 socket 判断是否可读，若可读客户端断开连接
            vector<DefaultClientSocket*>::iterator it = m_clientVec.begin();
            while(it != m_clientVec.end() && ret != 0){
                if(FD_ISSET((*it)->_getfd(),&set))
                {
                    string content = (*it)->read();
                    CHECK_STATE_EXCEPTION(content.empty());

                    Log(INFO) << "Client \"" << (*it)->getPeerAddr() << ':' << (*it)->getPeerPort() << "\" disconnect" << eol;

                    delete (*it);

                    --ret;
                    it = m_clientVec.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
}
