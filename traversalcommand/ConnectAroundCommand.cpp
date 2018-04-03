#include "ConnectAroundCommand.h"
#include "../socket/ReuseClientSocket.h"
#include "../include/SmartPointer.h"

#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/socket.h>

extern int errno;

using namespace Lib;

ClientSocket* ConnectAroundCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(DESTINY_PORT) ||
       !data.isMember(TRY_TIME) || !data.isMember(INCREMENT))
        return NULL;

    if(data.getInt(TYPE) != CONNECT_AROUND)
        return NULL;

    ip_type destiny_ip = data.getString(DESTINY_IP);
    port_type destiny_port = data.getInt(DESTINY_PORT);

    int try_time = data.getInt(TRY_TIME);
    port_type delta = data.getInt(INCREMENT);

    if(try_time < 2 || delta == 0)
        return NULL;

    fd_set set;
    FD_ZERO(&set);
    int maxfd = -1;

    SmartPointer<ReuseClientSocket> sockets[try_time];
    for(int i=0;i<try_time;++i){
        sockets[i].reset(new ReuseClientSocket());

        if(!sockets[i]->setNonBlock() || !sockets[i]->bind(ip,port))
            return NULL;

        FD_SET(sockets[i]->_getfd(),&set);
        if(maxfd < sockets[i]->_getfd())
            maxfd = sockets[i]->_getfd();
    }

    usleep(500000);

    for(int count=0;count < try_time && count * delta <= MAX_PORT - destiny_port;++count){
        if(sockets[count]->connect(destiny_ip.c_str(),destiny_port + count * delta,1)){
            int fd = sockets[count]->_getfd();
            sockets[count]->_invaild();
            return new ReuseClientSocket(fd);
        }else if(errno != EINPROGRESS){
            FD_CLR(sockets[count]->_getfd(),&set);
        }
    }

    struct timeval t = {5,0};
    int ret = select(maxfd+1,NULL,&set,NULL,&t);

    if(ret == -1){
        THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalServer::waitForClient");
    }else if(ret == 0){
        return NULL;
    }else{
        int error;
        socklen_t len = sizeof(int);
        for(int i=0;i<try_time;++i){
            if(FD_ISSET(sockets[i].get()->_getfd(),&set)){
                ::getsockopt(sockets[i].get()->_getfd(),SOL_SOCKET,SO_ERROR,&error,&len);

                if(error == 0){
                    int fd = sockets[i].get()->_getfd();
                    sockets[i]->_invaild();
                    return new ReuseClientSocket(fd);
                }
            }
        }
        return NULL;
    }
}
