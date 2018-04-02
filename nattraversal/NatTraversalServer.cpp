#include "../include/nattraversal/NatTraversalServer.h"
#include "../include/natchecker/NatCheckerServer.h"
#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../socket/DefaultClientSocket.h"
#include "../socket/DefaultServerSocket.h"
#include "../database/DefaultDataBase.h"
#include "../include/traversalcommand/TraversalCommand.h"
#include "NatTraversalCommon.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <sys/select.h>

#include <vector>
#include <thread>
#include <iostream>

using namespace std;
using namespace Lib;

vector<string> getLocalIps(){
    vector<string> ret;

    int s = socket(PF_INET, SOCK_DGRAM, 0);

    struct ifconf conf;
    char buff[BUFSIZ];
    conf.ifc_len = BUFSIZ;
    conf.ifc_buf = buff;

    if(ioctl(s, SIOCGIFCONF, &conf) == -1)
        return ret;

    int num = conf.ifc_len / sizeof(struct ifreq);
    struct ifreq *ifr = conf.ifc_req;

    for(int i=0;i < num;++i,++ifr){
        struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

        if(ioctl(s, SIOCGIFFLAGS, ifr) == -1)
            return ret;

        if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
            ret.push_back(string(inet_ntoa(sin->sin_addr)));
    }

    return ret;
}

bool NatTraversalServer::init(){
    vector<string> ips = getLocalIps();

    cout << "ip in this host:";
    for(vector<string>::size_type i=0;i<ips.size();++i)
        cout << " \"" << ips[i] << '\"';
    cout << endl;

    if(ips.size() < 2)
        return false;

    m_main_ip = ips[0];
    m_scondary_ip = ips[1];

    m_socket = new DefaultServerSocket();
    CHECK_NO_MEMORY_EXCEPTION(m_socket);

    if(!m_socket->bind(m_main_ip,SERVER_PORT) || !m_socket->listen(DEFAULT_LISTEN_NUM)){
        delete m_socket;
        m_socket = NULL;
        return false;
    }

    m_database = new DefaultDataBase<DataRecord>();
    CHECK_NO_MEMORY_EXCEPTION(m_database);

    m_database->addObserver(this);

    m_checker_server = new NatCheckerServer(m_main_ip,STUN_MAIN_PORT,m_scondary_ip,STUN_BACKUP_PORT);
    CHECK_NO_MEMORY_EXCEPTION(m_checker_server);

    if(!m_checker_server->setDataBase(m_database)){
        delete m_socket;
        m_socket = NULL;
        delete m_database;
        m_database = NULL;
        delete m_checker_server;
        m_checker_server = NULL;
        return false;
    }

    m_user_manager = new DefaultDataBase<UserRecord>();
    CHECK_NO_MEMORY_EXCEPTION(m_user_manager);

    return true;
}

void NatTraversalServer::term(){
    if(m_checker_server)
        delete m_checker_server;
    if(m_database)
        delete m_database;
    if(m_socket)
        delete m_socket;
    if(m_user_manager)
        delete m_user_manager;
}

void NatTraversalServer::notify(void* msg){
    string identifier((char*)msg);
    m_semaphore[identifier].release();
}

void NatTraversalServer::getAndStoreIdentifier(DefaultClientSocket *socket){
    if(socket == NULL)
        return;

    TransmissionProxy proxy(socket);
    TransmissionData data = proxy.read();

    if(!data.isMember(IDENTIFIER))
        return;

    cout << "Client \"" << data[IDENTIFIER] << "\" login in" << endl;

    m_user_manager->addRecord(UserRecord(data.getString(IDENTIFIER),socket));
}

void NatTraversalServer::waitForDataBaseUpdate(const std::string& identifier){
    m_semaphore[identifier].request();
}

void NatTraversalServer::handle_connect_request(NatTraversalServer *m_traversal_server,const std::string& identifier, DefaultClientSocket *socket){
    TransmissionProxy proxy(socket);
    TransmissionData data;

    UserManager *userManager = m_traversal_server->m_user_manager;
    
    // 处理一次客户端发来时的请求与其他对等端的连接请求：首先读取信息，若读取不到东西，则表示套接字已经关闭，关闭套接字并在用户登录表中删除该
    // 用户。若读取得到信息，则表示是客户端发来消息，有两种消息：一种是通知服务端是否允许接收他人的连接，另一种是请求发起与对等端的连接
    // 前者包含 READ_TO_CONNECT 字段，表示设置是否允许接收他人的连接，服务器若检测到有这个字段，则在用户表中设置该用户的可连接字段；
    // 后者包含 PEER_HOST 字段，表示要发起连接的对等方表示符号，读取到 PEER_HOST 后判断对等方是否已经登录且已经准备好连接，若没有，则返回错误信息
	// 若准备好连接，则服务端向双方发送 NAT 类型检查信令，接着等待这边 NAT 类型检查服务器得到双方的 NAT 类型，服务器通过判断两者的类型，向双方
	// 发出穿越操作指令，使两者进行 P2P 连接

    data = proxy.read();

    if(data.empty()){	// 读取到的内容是空的，表示连接已关闭
        delete socket;
        userManager->removeRecord(identifier);

        cout << "Client \"" << identifier << "\" login out" << endl;
    }else{
        string peer_identifier;

        vector<string> members = data.getMembers();
        for(vector<string>::size_type i=0;i<members.size();++i)
            cout << members[i] << endl;

        if(data.isMember(READY_TO_CONNECT))	// 内容包括 READY_TO_CONNECT ，设置用户的能否来连接字段
        {
            bool isReady = data.getBool(READY_TO_CONNECT);
            userManager->getRecord(identifier).setReady(isReady);
        }
        else if(data.isMember(PEER_HOST))	// 内容包括 PEER_HOST ，进行穿透操作
        {
            TraversalCommand::Types types;
            DataRecord record_a;
            DataRecord record_b;

            nat_type natType_a;
            nat_type natType_b;

            bool isReady = (*userManager)[identifier].isReady();	// 先设置请求连接方为不允许建立连接，暂时屏蔽其他客户端对该客户端的连接请求
            if(isReady)											// P2P 连接建立成功后再恢复
                (*userManager)[identifier].setReady(false);

            peer_identifier = data.getString(PEER_HOST);
            if(!userManager->hasRecord(peer_identifier) || !(*userManager)[peer_identifier].isReady())
                goto r;

            data.clear();
            data.add(CAN_CONNECT,true);
            data.add(STUN_IP,m_traversal_server->m_main_ip);
            data.add(STUN_PORT,STUN_MAIN_PORT);

            if(!proxy.write(data))
                goto r;

            data.remove(CAN_CONNECT);
            proxy.setSocket(userManager->getRecord(peer_identifier).getClientSocket());
            proxy.write(data);

            m_traversal_server->waitForDataBaseUpdate(identifier);
            m_traversal_server->waitForDataBaseUpdate(peer_identifier);

            record_a = m_traversal_server->m_database->getRecord(identifier);
            record_b = m_traversal_server->m_database->getRecord(peer_identifier);

            natType_a = record_a.getNatType();
            natType_b = record_b.getNatType();

            types = GetTraversalType(natType_a,natType_b);

            data = GetTraversalData(types[0],natType_b,record_b.getExtAddress().ip,record_b.getExtAddress().port);
            data.add(CAN_CONNECT,true);
            if(!proxy.write(data))
                goto r;

            data = GetTraversalData(types[1],natType_a,record_a.getExtAddress().ip,record_a.getExtAddress().port);
            proxy.setSocket(userManager->getRecord(peer_identifier).getClientSocket());
            proxy.write(data);

            return;
r:
            data.clear();
            data.add(CAN_CONNECT,false);
            proxy.write(data);

            if(isReady)
                (*userManager)[identifier].setReady(true);
        }
        else
        {
            // TODO client send something can not recognize
        }
    }
}

void handle_nat_check_request(NatCheckerServer* checker_server){
    checker_server->waitForClient();
}

void NatTraversalServer::waitForClient(){
    CHECK_OPERATION_EXCEPTION(m_checker_server);

	// 服务器先开启 NatChecker 服务器的监听，在新线程处理 NAT 类型检测的请求，在初始化时已经设置了 DataBase 进去，会自动地添加 NAT 类型记录到
	// DataBase 中，以便主服务器使用，DataBase 注意线程安全
    thread(handle_nat_check_request,m_checker_server).detach();

    fd_set set;
    int ret,maxfd;
    while(1){                                   // Review : pay attention to the number of socket
        FD_ZERO(&set);

        FD_SET(m_socket->_getfd(),&set);		// 把服务器 Socket 和所有接受到的客户端连接 Socket 都添加到 set 中
        maxfd = m_socket->_getfd();

        int fd;
        vector<string> identifiers = m_user_manager->getMembers();
        for(vector<string>::size_type i=0;i<identifiers.size();++i){
            fd = (*m_user_manager)[identifiers[i]].getClientSocket()->_getfd();

            FD_SET(fd,&set);

            if(maxfd < fd)
                maxfd = fd;
        }

		// 使用 select 进行 I/O 多路转接，避免为客户端开启太多线程
        ret = select(maxfd+1,&set,NULL,NULL,NULL);

        if(ret == -1)
        {
            THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalServer::waitForClient");
        }else if(ret == 0)
        {
            THROW_EXCEPTION(ErrorStateException,"select timeout in NatTraversalServer::waitForClient");
        }else
        {
        	// 如果服务器端可读，亦即有新客户端请求连接，则从连接中读取发送过来的标识符，保存用户的标识符、socket、以及 isReady 标志(默认为假)
            if(FD_ISSET(m_socket->_getfd(),&set))
            {
                ClientSocket *client = m_socket->accept();

                DefaultClientSocket *defaultSocket = dynamic_cast<DefaultClientSocket*>(client);
                CHECK_STATE_EXCEPTION(defaultSocket);

                getAndStoreIdentifier(defaultSocket);

                --ret;
            }

			// 遍历存储的客户端 socket 判断是否可读，若可读表示有客户端发送消息，请求与其他对等端发起连接，为处理每个请求建立一条临时的线程
			// 多线程并发处理多个用户的请求提高处理效率，然后等待所有处理的线程结束，重新调用 select 监听所有 socket
			// 注意，不可以多线程并发处理时主线程重新循环调用 select，因为处理线程正在与客户端通信，select 会认为其可读，以为发起新的连接请求

            vector<thread> vecThread;
            DefaultClientSocket *socket;

            vector<string> identifiers = m_user_manager->getMembers();
            for(vector<string>::size_type i=0;i<identifiers.size() && ret != 0;++i){
                socket = (*m_user_manager)[identifiers[i]].getClientSocket();

                if(FD_ISSET(socket->_getfd(),&set)){
                    vecThread.push_back(thread(handle_connect_request,this,identifiers[i],socket));

                    --ret;
                }
            }

            for(vector<thread>::size_type i=0;i<vecThread.size();++i)
                vecThread[i].join();
        }
    }
}
