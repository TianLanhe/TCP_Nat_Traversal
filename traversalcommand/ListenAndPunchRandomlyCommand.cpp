#include "ListenAndPunchRandomlyCommand.h"
#include "../include/socket/ServerSocket.h"
#include "../include/socket/ClientSocket.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/SmartPointer.h"
#include "../socket/ReuseServerSocket.h"

#include <sys/select.h>
#include <thread>
#include <cstdlib>
#include <vector>
#include <algorithm>

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

    if(RAND_MAX - start >= length)
        length = RAND_MAX - start;

    return _nrand(length) + start;
}

void ListenAndPunchRandomlyCommand::punching(ListenAndPunchRandomlyCommand* command, vector<ClientSocket*> clients, int try_time, const ip_type &destiny_ip, port_type destiny_port)
{
    while(command->shouldPunch){
        for(int i=0;i<try_time;++i)
            clients[i]->connect(destiny_ip,destiny_port,1);
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

    vector<SmartPointer<ReuseServerSocket> > servers(try_time);
    ReuseServerSocket *reuseSocket;

    vector<SmartPointer<ClientSocket> > clients(try_time);
    vector<ClientSocket*> ptrClients(try_time);

    vector<port_type> ports;
    port_type random_port;

    struct timeval t = {5,0};
    fd_set set;
    FD_ZERO(&set);
    int maxfd = -1;
    int fd;

    for(int i =0;i < try_time ;++i){
        reuseSocket = dynamic_cast<ReuseServerSocket*>(ReuseSocketFactory::GetInstance()->GetServerSocket());
        if(reuseSocket == NULL)
            return NULL;

        servers[i].reset(reuseSocket);

        clients[i].reset(ReuseSocketFactory::GetInstance()->GetClientSocket());
        ptrClients[i] = clients[i].get();

        do{
            random_port = _getRandomNum(1024,64512);
        }while(find(ports.begin(),ports.end(),random_port) != ports.end());
        ports.push_back(random_port);

        if(!clients[i]->setNonBlock() || !clients[i]->bind(ip,random_port))
            return NULL;

        if(!servers[i]->bind(ip,random_port))
            return NULL;

        if(!servers[i]->listen(1))
            return NULL;

        fd = reuseSocket->_getfd();
        FD_SET(fd,&set);
        if(fd > maxfd)
            maxfd = fd;
    }

    thread punch_thread(punching,this,ptrClients,try_time,destiny_ip,destiny_port);

    int ret = select(maxfd+1,&set,NULL,NULL,&t);

    if(ret == -1){
        THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalServer::waitForClient");
    }else if(ret == 0){
        shouldPunch = false;
        punch_thread.join();
        return NULL;
    }else{
        CHECK_STATE_EXCEPTION(ret == 1);

        ClientSocket *client;
        for(int i=0;i<try_time;++i){
            if(FD_ISSET(servers[i]->_getfd(),&set)){
                client = servers[i]->accept();
                break;
            }
        }

        shouldPunch = false;
        punch_thread.join();

        return client;
    }
}
