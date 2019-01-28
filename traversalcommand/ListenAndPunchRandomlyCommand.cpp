#include "ListenAndPunchRandomlyCommand.h"
#include "../include/SmartPointer.h"
#include "../socket/ReuseServerSocket.h"
#include "../socket/ReuseClientSocket.h"

#include <sys/select.h>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sys/socket.h>

using namespace std;
using namespace Lib;

int ListenAndPunchRandomlyCommand::_nrand(int n){
    if(n < 0)
        n = 0;
    else if(n > RAND_MAX)
        n = RAND_MAX;

    int r;
    int bucket_size = RAND_MAX / n;
    do{
        r = rand() / bucket_size;
    }while( r >= n );

    return r;
}

int ListenAndPunchRandomlyCommand::_getRandomNum(int start,int length){
    if(start < 0)
        start = 0;
    else if(start > RAND_MAX)
        start = RAND_MAX;

    if(RAND_MAX - start < length)
        length = RAND_MAX - start;

    return _nrand(length) + start;
}

void ListenAndPunchRandomlyCommand::punching(ListenAndPunchRandomlyCommand* command, vector<ClientSocket*> clients, int fd, const ip_type &destiny_ip, port_type destiny_port)
{
    vector<ClientSocket*>::size_type size = clients.size();
    while(command->shouldPunch){
        for(int i=0;i<size;++i){
            if(clients[i]->connect(destiny_ip,destiny_port)){
                ::shutdown(fd,SHUT_RD);
                command->shouldPunch = false;
                break;
            }
        }
    }
}

ClientSocket* ListenAndPunchRandomlyCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(DESTINY_PORT) || !data.isMember(TRY_TIME))
        return NULL;

    if(data.getInt(TYPE) != LISTEN_BY_PUNCHING_SOME_HOLE)
        return NULL;

    ip_type destiny_ip = data.getString(DESTINY_IP);
    port_type destiny_port = data.getInt(DESTINY_PORT);

    int try_time = data.getInt(TRY_TIME);

    if(try_time < 2)
        return NULL;

	// 存储 try_time 个 client socket 用于打洞，绑定不同的源端口以保证在 NAT 上映射出不同的外部端口
    vector<SmartPointer<ClientSocket> > clients(try_time);
    vector<ClientSocket*> ptrClients(try_time);

	// 存储 try_time 个 server socket 在本地监听，每个 server socket 对应监听上面 client socket 绑定的源端口
    vector<SmartPointer<ReuseServerSocket> > servers(try_time);
    ReuseServerSocket *reuseSocket;

    vector<port_type> ports;
    port_type random_port;

    fd_set set;
    FD_ZERO(&set);
    int maxfd = -1;
    int fd;

	srand(time(NULL));
    for(int i=0;i < try_time ;++i){
        reuseSocket = new ReuseServerSocket();

        servers[i].reset(reuseSocket);

        clients[i].reset(new ReuseClientSocket());
        ptrClients[i] = clients[i].get();

        do{
            random_port = _getRandomNum(1024,64512);
        }while(find(ports.begin(),ports.end(),random_port) != ports.end());
        ports.push_back(random_port);

        if(!clients[i]->setNonBlock() || !clients[i]->bind(ip,random_port))
            return NULL;

        if(!servers[i]->bind(ip,random_port))
            return NULL;

        if(!servers[i]->listen(LISTEN_NUMBER))
            return NULL;

        fd = reuseSocket->_getfd();
        FD_SET(fd,&set);
        if(fd > maxfd)
            maxfd = fd;
    }

    thread punch_thread(ListenAndPunchRandomlyCommand::punching,this,ptrClients,servers[0]->_getfd(),destiny_ip,destiny_port);

    struct timeval t = SELECT_WAIT_TIME;
    int ret = select(maxfd+1,&set,NULL,NULL,&t);

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
            shouldPunch = false;

            for(int i=0;i<try_time;++i){
                if(FD_ISSET(servers[i]->_getfd(),&set)){
                    ret = servers[i]->accept();
                    break;
                }
            }

            punch_thread.join();
        }else{
            for(int i=0;i<try_time;++i){
                if(clients[i]->isConnected()){
                    ReuseClientSocket *client = dynamic_cast<ReuseClientSocket*>(clients[i].get());
                    if(client == NULL)
                        return NULL;

                    client->setNonBlock(false);

                    ret = new ReuseClientSocket(client->_getfd());
                    client->_invalid();
                    break;
                }
            }

            punch_thread.join();
        }

        return ret;
    }
}
