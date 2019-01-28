#include "ListenAndPunchCommand.h"
#include "../include/socket/ServerSocket.h"
#include "../include/socket/ClientSocket.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/SmartPointer.h"
#include "../socket/ReuseServerSocket.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <thread>

using namespace std;
using namespace Lib;

void ListenAndPunchCommand::punching(ListenAndPunchCommand *command, ClientSocket *socket,  int serverFd, const ip_type& destiny_ip, port_type destiny_port)
{
    while(command->shouldPunch){
        if(!socket->isConnected()){
            socket->connect(destiny_ip,destiny_port);
        }else{	// 如果发生同时打开，连接上了，这里关闭 server socket 的读通道，且将 shouldPunch 置为 false
            command->shouldPunch = false;
            shutdown(serverFd,SHUT_RD);
            break;
        }
        command->sleep(PUNCHING_INTEVAL);
    }
}

ClientSocket* ListenAndPunchCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(DESTINY_PORT))
        return NULL;

    if(data.getInt(TYPE) != LISTEN_BY_PUNCHING_A_HOLE)
        return NULL;

    SmartPointer<ServerSocket> server(ReuseSocketFactory::GetInstance()->GetServerSocket());
    ClientSocket *client = ReuseSocketFactory::GetInstance()->GetClientSocket();

    if(!client->setNonBlock() || !client->bind(ip,port)){
        delete client;
        return NULL;
    }

    if(!server->bind(ip,port)){
        delete client;
        return NULL;
    }

    if(!server->listen(LISTEN_NUMBER)){
        delete client;
        return NULL;
    }

    ReuseServerSocket *reuseSocket = dynamic_cast<ReuseServerSocket*>(server.get());
    if(reuseSocket == NULL){
        delete client;
        return NULL;
    }

    struct timeval t = SELECT_WAIT_TIME;
    fd_set set;
    FD_ZERO(&set);
    FD_SET(reuseSocket->_getfd(),&set);

	// 这里可能发生一种很特殊的情况：TCP同时打开，即没有 server socket 进行监听，而是两个 socket 同时 connect，然后进入连接状态
	// 这里是一个 server socket 进行监听，然后再次线程往目的地址不断 connect 打洞，有可能发生同时打开(亲测两个对称型NAT大概率会发生)用于
	// 打洞的 socket 跟对方连接上了。这里的处理方法是在线程中判断 socket 的连接状态，如果连接成功了，则手动 shutdown 在监听的 server socket 以
	// 唤醒被 select 阻塞的主线程。注意这里不能 close server socket 以唤醒 select
    thread punch_thread(ListenAndPunchCommand::punching,this,client,reuseSocket->_getfd(),data.getString(DESTINY_IP),data.getInt(DESTINY_PORT));

    int ret = select(reuseSocket->_getfd()+1,&set,NULL,NULL,&t);

    if(ret == -1){
        THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalServer::waitForClient");
    }else if(ret == 0){
        shouldPunch = false;
        punch_thread.join();
        return NULL;
    }else{
        CHECK_STATE_EXCEPTION(ret == 1);

        ClientSocket *ret = NULL;

        if(shouldPunch){
            ret = server->accept();

            shouldPunch = false;
            punch_thread.join();

            delete client;
        }else{
            ret = client;
            ret->setNonBlock(false);
            punch_thread.join();
        }

        return ret;
    }
}
