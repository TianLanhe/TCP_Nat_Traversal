#include "../include/nattraversal/NatTraversalServer.h"
#include "../include/natchecker/NatCheckerServer.h"
#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../socket/DefaultClientSocket.h"
#include "../socket/DefaultServerSocket.h"
#include "../database/DefaultDataBase.h"
#include "../include/traversalcommand/TraversalCommand.h"
#include "NatTraversalCommon.h"
#include "../include/Log.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <thread>

extern int errno;

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

    ::close(s);
    return ret;
}

bool NatTraversalServer::init(){
    vector<string> ips = getLocalIps();

    string content;
    for(vector<string>::size_type i=0;i<ips.size();++i)
        content.append('\"' + ips[i] + "\" ");
    //log("NatTraversalServer: ","Local IP: ",content);

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

	// 将 NatTraversalServer 作为观察者添加到数据库中，当 NAT 数据库添加或删除一条记录时，会通知主服务器
    m_database->addObserver(this);

    m_checker_server = new NatCheckerServer(m_main_ip,STUN_MAIN_PORT,m_scondary_ip,STUN_SECONDARY_PORT);
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

void NatTraversalServer::getAndStoreIdentifier(DefaultClientSocket *socket){
    if(socket == NULL)
        return;

    TransmissionProxy proxy(socket);
    TransmissionData data = proxy.read();

    if(!data.isMember(IDENTIFIER)){
    	delete socket;
        return;
    }

    //log("NatTraversalServer: ","Client \"",data[IDENTIFIER],"\" login (IP: ",socket->getPeerAddr(),")");

    m_user_manager->addRecord(UserRecord(data.getString(IDENTIFIER),socket));
}

void NatTraversalServer::notify(void* msg){
    string identifier((char*)msg);
    m_semaphore[identifier].release();
}

void NatTraversalServer::timer(NatTraversalServer *m_traversal_server,const string& identifier){
    sleep(TIME_TO_WAIT_FOR_CHECKING_NAT_TYPE);

    m_traversal_server->m_semaphore[identifier].release();
}

void NatTraversalServer::waitForDataBaseUpdate(const std::string& identifier){
    //thread t(timer,this,identifier);
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

    if(data.empty())
    {	// 读取到的内容是空的，表示连接已关闭
        delete socket;
        userManager->removeRecord(identifier);

        //log("NatTraversalServer: ","Client \"",identifier,"\" logout");
    }
    else
    {
        if(data.isMember(READY_TO_CONNECT))	// 内容包括 READY_TO_CONNECT ，设置用户的能否连接字段
        {
            bool isReady = data.getBool(READY_TO_CONNECT);
            userManager->getRecord(identifier).setReady(isReady);

            if(isReady){
                //log("NatTraversalServer: ","Client \"",identifier,"\" set ready to accept connecting");
            }else{
                //log("NatTraversalServer: ","Client \"",identifier,"\" set unready to accept connecting");
            }
        }
        else if(data.isMember(PEER_HOST))	// 内容包括 PEER_HOST ，进行穿透操作
        {
            TraversalCommand::Types types;
            vector<DataRecord> records(2);
            vector<nat_type> natTypes(2);
            vector<string> identifiers(2);
            vector<ClientSocket*> sockets(2);
            int firstSend = 0;

            // 先设置请求连接方为不允许建立连接，暂时屏蔽其他客户端对该客户端的连接请求,P2P 连接建立成功后再恢复
            bool isReady = (*userManager)[identifier].isReady();
            if(isReady){
                (*userManager)[identifier].setReady(false);
                //log("NatTraversalServer: ","Client \"",identifier,"\" is set to be unready");
            }

            string peer_identifier = data.getString(PEER_HOST);
            //log("NatTraversalServer: ","Client \"",identifier,"\" want to connect with Client \"",peer_identifier,"\"");

            // 判断准备连接的对等方是否已经登录以及是否允许连接
            if(!userManager->hasRecord(peer_identifier) || !(*userManager)[peer_identifier].isReady()){
                //log("NatTraversalServer: ","Client \"",peer_identifier,"\" is not logged in");
                goto r;
            }

			// 对等方已登录且允许连接，向客户端发送允许连接信令，同时发送 STUN 的地址，让其进行 NAT 类型检测
            data.clear();
            data.add(CAN_CONNECT,true);
            data.add(STUN_IP,m_traversal_server->m_main_ip);
            data.add(STUN_PORT,STUN_MAIN_PORT);

            if(!proxy.write(data))
                goto r;

            //log("NatTraversalServer: ","Waiting for Client \"",identifier,"\" to detect the nat type");

			// 向对等方发送 STUN 的地址，让其进行 NAT 类型检测(对等方已经随时监听着)
            data.remove(CAN_CONNECT);
            proxy.setSocket(userManager->getRecord(peer_identifier).getClientSocket());
            proxy.write(data);
            //log("NatTraversalServer: ","Waiting for Client \"",peer_identifier,"\" to detect the nat type");

			// Review: 这里会阻塞等待 NAT 类型检测完成，如果客户端和对等方发送出现问题，这里整个服务器会永远阻塞出不来
			// 等待客户端 NAT 类型检测完成的信号
            m_traversal_server->waitForDataBaseUpdate(identifier);
            //log("NatTraversalServer: ","Client \"",identifier,"\" finish NAT type detection");

			// 等待对方等 NAT 类型检测完成的信号
            m_traversal_server->waitForDataBaseUpdate(peer_identifier);
            //log("NatTraversalServer: ","Client \"",peer_identifier,"\" finish NAT type detection");

			// 两方 NAT 类型检测完成，从数据库中取出两方的外部地址与 NAT 类型信息
            records[0] = m_traversal_server->m_database->getRecord(identifier);
            records[1] = m_traversal_server->m_database->getRecord(peer_identifier);

            natTypes[0] = records[0].getNatType();
            natTypes[1] = records[1].getNatType();

			identifiers[0] = identifier;
			identifiers[1] = peer_identifier;
			
			sockets[0] = socket;
			sockets[1] = userManager->getRecord(peer_identifier).getClientSocket();

			// 根据 NAT 类型信息判断两方该采取什么操作
            types = GetTraversalType(natTypes[0],natTypes[1]);

            static const char* typeStr[]={"","connect directly","connect nearby","connect randomly",
                                          "listen directly","listen and punch a hole","listen and punch some hole"};

            //log("NatTraversalServer: ",identifiers[0],": ",typeStr[types[0]],", ",identifiers[1],": ",typeStr[types[1]]);
            
            // 这里判断了一下哪方是 Listen 的，给 Listen 的一方先发送数据，以使 Listen 的一方能在对方 Connect 之前准备好，Connect 客户端那边也会
            // 休眠一段时间(0.5s)以保证 Listen 一方准备好
            if(types[0] == TraversalCommand::LISTEN_BY_PUNCHING_A_HOLE ||
                types[0] == TraversalCommand::LISTEN_BY_PUNCHING_SOME_HOLE ||
                types[0] == TraversalCommand::LISTEN_DIRECTLY)
            {
                firstSend = 0;
            }
            else
            {
                firstSend = 1;
            }

            data = GetTraversalData(types[firstSend],natTypes[1-firstSend],records[1-firstSend].getExtAddress().ip,records[1-firstSend].getExtAddress().port);
            proxy.setSocket(sockets[firstSend]);
            if(!proxy.write(data))
                goto r;

            //log("NatTraversalServer: ","send traversal command to Client \"",identifiers[firstSend],"\"");

            data = GetTraversalData(types[1-firstSend],natTypes[firstSend],records[firstSend].getExtAddress().ip,records[firstSend].getExtAddress().port);
            proxy.setSocket(sockets[1-firstSend]);
            proxy.write(data);

            //log("NatTraversalServer: ","send traversal command to Client \"",identifiers[1-firstSend],"\"");

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
            //log("NatTraversalServer: ","Client \"",identifier,"\" send something can not recognize");
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
    while(1){
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

        // 使用 select 进行 I/O 多路转接，避免为客户端开启太多线程。Review 缺点：必须等一批客户端处理完毕后才能循环下一次 select，万一阻塞就GG
        ret = select(maxfd+1,&set,NULL,NULL,NULL);

        if(ret == -1)
        {
            if(errno == EINTR){
                //log("NatTraversalServer: ","Warning : NatTraversalServer is interrupted at function select in NatTraversalServer::waitForClient");
            }else{
                THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalServer::waitForClient");
            }
        }
        else if(ret == 0)
        {
            THROW_EXCEPTION(ErrorStateException,"select timeout in NatTraversalServer::waitForClient");
        }
        else
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
