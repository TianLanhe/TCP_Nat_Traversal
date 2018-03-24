#include "../include/natchecker/NatCheckerServer.h"
#include "../include/socket/ClientSocket.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"

#include <thread>
#include <iostream>
#include <string>

using namespace std;
using namespace Lib;

NatCheckerServer::NatCheckerServer(const string& main_addr, port_type main_port, const string& another_addr, port_type another_port)
    :m_main_addr(main_addr),m_main_port(main_port),
      m_another_addr(another_addr),m_another_port(another_port){
    m_main_server = ReuseSocketFactory::GetInstance()->GetServerSocket();
    CHECK_NO_MEMORY_EXCEPTION(m_main_server);
}

NatCheckerServer::~NatCheckerServer(){
    if(m_main_server)
        delete m_main_server;
}

bool NatCheckerServer::setListenNum(size_t num){
    if(!m_main_server)
        return false;

    if(!m_main_server->isBound() && !m_main_server->bind(m_main_addr,m_main_port))
        return false;

    return m_main_server->listen(num);
}

void NatCheckerServer::handle_request(NatCheckerServer *server,ClientSocket* client){
    TransmissionProxy proxy(client);
    TransmissionData data = proxy.read();

	// 先取出 Client 发送过来的本机地址，对比这边观察到的外部地址，如果外部地址与 Client 发送过来的它那边观察到的地址相同，
	// 表示 Client 处于公网中，可以停止 NAT 类型检测。如果不相同，则先进行 NAT 的 Filter 类型检测(Client那边已经开启了监听，等待连接)，
	// Filter 具体检测过程见下。检测完成后，接着要 Client 配合探测 NAT 的 MAP 类型以及端口映射规律，即不断地让 Client 连接服务器要它连接的端口
	// 服务器检查它连接过来的外部地址，若不明确类型及规律，继续让 Client 连接新端口，直到 STUN 服务器探测出需要的了，就发送停止探测的信息
	// 服务器可将整个过程获得到的信息存储起来以便其他用途
	
    if(!data.isMember(LOCAL_IP) || data.isMember(LOCAL_PORT))
        return;

    string local_ip = data.getString(LOCAL_IP);
    port_type local_port = data.getInt(LOCAL_PORT);

    string ext_ip = client->getAddr();
    port_type ext_port = client->getPort();

    cout << "local_ip: " << local_ip << endl;
    cout << "local_port: " << local_port << endl;

    cout << "ext_ip: " << ext_ip << endl;
    cout << "ext_port: " << ext_port << endl;

    if(local_ip == ext_ip && local_port != ext_port){
        // TODO 这是很奇怪的现象，IP 相同端口却改变了
    }

    if(local_ip == ext_ip && local_port == ext_port)	// 内外地址相同，处于公网中，停止检测
    {
        data.clear();
        data.add(CONTINUE,false);
        proxy.write(data);
    }
    else
    {   // 内外地址不同，存在 NAT，进行 NAT 的 Filter 类型检测 ： 先从 IP2:Port1 对客户端外网地址发起连接
    	// 若连接成功，表示 Client 在 NAT 留下的映射其他 IP 也能通过，因此 NAT 属于 EndPoint Independent 的 Filter 规则
		// 若连接失败，则从 IP1:Port2 对客户端外网地址发起连接，若连接成功，表示 Client 在 NAT 留下的映射只有同一个 IP 才能通过，
		// 因此 NAT 属于 Address Dependent 的 Filter 规则；若连接失败，表示相同 IP 不同 Port 的不能通过，
		// 因此 NAT 属于 Address And Port Denpendent 的Filter规则
        ClientSocket *c = ReuseSocketFactory::GetInstance()->GetClientSocket();
        if(!c->bind(server->m_another_addr,server->m_main_port))
            return;

        bool canConnect = c->connect(ext_ip,ext_port);
        filter_type filterType;
        if(canConnect)
        {
            filterType = ENDPOINT_INDEPENDENT;
        }
        else
        {
        	// 若无法连接上，则关闭原来的 Client Socket，重新打开一个，绑定地址为 IP1:Port2，并尝试连接
            if(!c->close() || !c->open() || !c->bind(server->m_main_addr,server->m_another_port))
                return;

            canConnect = c->connect(ext_ip,ext_port);

            if(canConnect)
                filterType = ADDRESS_DEPENDENT;
            else
                filterType = ADDRESS_AND_PORT_DEPENDENT;
        }

        delete c;
        c = NULL;

		// 为接下来检测 NAT 的 MAP 类型做准备，先在 IP2:Port1 监听，通知 Client 连接，若这一次(第2次)连接所观察到的地址与上一次(第1次)相同
		// 表示 NAT 对 Client 发到任何地址的包都映射到同一个外网地址上，说明 NAT 属于 EndPoint Independent 的 Map 规则，
		// 若不相同，则监听 IP2:Port2 ，并通知 Client 连接到此地址，若这一次(第3次)连接所观察到的地址与第2次的端口相同，
		// 则表示 NAT 对 Client 发到同一个 IP 的包都映射到同一个外网地址上，说明 NAT 属于 Address Dependent 的 Map 规则，
		// 记录第3次连接的 Port 与第1次连接的 Port 的差值，此为端口变化的规律（这里存在两次观察只有一个差值无法判断规律是否正确的不足）
		// 并返回信息给 Client，通知其 NAT 的 Map 类型以及告诉它停止检测
		// 若这一次(第3次)连接所观察到的地址与第2次的端口不同，表示 NAT 对 Client 发到任意不同地址的包都映射到新的地址上
		// 说明 NAT 属于 Address And Port Dependent 的 Map 规则
		// 这时已经掌握了三个不同端口相同 IP 的地址了，根据差值判断端口变化规律，若两次差值相同，则确定此差值，并通知 Client
		// 若两次差值不同，则继续循环，让 Client 继续连接，直至这边观察到连续两次差值相同或循环次数已经达到阈值为止
		
        ServerSocket *s = ReuseSocketFactory::GetInstance()->GetServerSocket();
        if(!s->bind(server->m_another_addr,server->m_main_port) || !s->listen(DEFAULT_LISTEN_NUM))
            return;

        data.clear();
        data.add(CONTINUE,true);
		data.add(FILTER_TYPE,filterType);
        data.add(EXTERN_IP,ext_ip);
        data.add(EXTERN_PORT,ext_port);
        data.add(CHANGE_IP,server->m_another_addr);
        data.add(CHANGE_PORT,server->m_main_port);
        
        proxy.write(data);		// STUN 服务器将信息发送给 Client，并等待 Client 连接

        c = s->accept();

        string ext_ip2 = c->getAddr();
        port_type ext_port2 = c->getPort();
        
   	 	cout << "ext_ip2: " << ext_ip2 << endl;
    	cout << "ext_port2: " << ext_port2 << endl;

        if(ext_ip2 != ext_ip){
            // TODO 假设 NAT 只有一个对外 IP 或 同一个内网主机向外通信时肯定会转换到同一个 IP (也许会改变端口)
        }


        if(ext_ip2 == ext_ip && ext_port2 == ext_port) // 第2次的外网地址与第1次的相同
        {
            data.clear();
            data.add(MAP_TYPE,ENDPOINT_INDEPENDENT);
            data.add(CONTINUE,false);

            proxy.setSocket(c);
            proxy.write(data);

            delete s;
            s = NULL;
            
            delete c;
            c = NULL;
            
            // TODO 端口增量为0
        }
        else	// 第2次的外网地址与第1次的不同，需要进一步判断是 Address Dependent 还是 Address And Port Dependent
        {
            data.clear();
            data.add(CONTINUE,true);
            data.add(CHANGE_IP,server->m_another_addr);
            data.add(CHANGE_PORT,server->m_another_port);

            if(!s->close() || !s->open() || !s->bind(server->m_another_addr,server->m_another_port) || !s->listen(DEFAULT_LISTEN_NUM))
                return;

            proxy.setSocket(c);
            proxy.write(data);

            // TODO 这里之前连接的c要如何处理，先测试一下释放跟不释放时所观察到的地址是否有不同，现在先让它泄漏吧
            c = s->accept();

        	string ext_ip3 = c->getAddr();
        	port_type ext_port3 = c->getPort();
        
   	 		cout << "ext_ip3: " << ext_ip3 << endl;
    		cout << "ext_port3: " << ext_port3 << endl;

            if(ext_ip2 != ext_ip3){
                // TODO 假设 NAT 只有一个对外 IP 或 同一个内网主机向外通信时肯定会转换到同一个 IP (也许会改变端口)
            }

            if(ext_ip3 == ext_ip2 && ext_port3 == ext_port2) // 第3次的外网地址与第2次的相同
            {
                data.clear();
                data.add(MAP_TYPE,ADDRESS_DEPENDENT);
                data.add(CONTINUE,false);

                proxy.setSocket(c);
                proxy.write(data);

                delete s;
                s = NULL;

                // TODO c先让它泄漏吧
                
                // TODO 端口增量为 ext_port2 - ext_port
            }
            else // 第3次与第2次的端口不同，表示时 Address And Port Dependent
            {
            	int delta_pre = ext_port2 - ext_port;
            	int delta_cur = ext_port3 - ext_port2;
            	int try_time = 0;
                port_type ext_port_pre = ext_port3;
            	
            	while( delta_pre != delta_cur && try_time++ < server->_getMaxTryTime()){
                    data.clear();
                    data.add(CONTINUE,true);
                    data.add(CHANGE_IP,server->m_another_addr);
                    data.add(CHANGE_PORT,server->m_another_port + try_time);

                    if(!s->close() || !s->open() ||
                            !s->bind(server->m_another_addr,server->m_another_port + try_time) ||
                            !s->listen(DEFAULT_LISTEN_NUM))
                        return;

                    proxy.setSocket(c);
                    proxy.write(data);

                    // TODO 这里之前连接的c要如何处理，先测试一下释放跟不释放时所观察到的地址是否有不同，现在先让它泄漏吧
                    c = s->accept();

                    string ext_ip_n = c->getAddr();
                    port_type ext_port_n = c->getPort();

                    cout << "ext_ip_n: " << ext_ip_n << endl;
                    cout << "ext_port_n: " << ext_port_n << endl;

                    delta_pre = delta_cur;
                    delta_cur = ext_port_n - ext_port_pre;
                    ext_port_pre = ext_port_n;
            	}

                delete s;
                s = NULL;

                // TODO how to deal with c
            	
                data.clear();
                data.add(MAP_TYPE,ADDRESS_AND_PORT_DEPENDENT);
                data.add(CONTINUE,false);

                proxy.setSocket(c);
                proxy.write(data);

            	if( try_time == server->_getMaxTryTime() ){
            		// TODO 端口随机变化
            	}else{
            		// TODO 设置增量为 delta_cur;
            	}
            }
        }
    }
}

void NatCheckerServer::waitForClient(){
    if(!m_main_server->isListen() && !setListenNum(DEFAULT_LISTEN_NUM))
        THROW_EXCEPTION(InvalidOperationException,"bind or listen error");

    ClientSocket *client;
    while(client = m_main_server->accept()){
        thread(handle_request,this,client).detach();
    }
}
