#include "ConnectRandomlyCommand.h"
#include "../socket/ReuseClientSocket.h"
#include "../include/SmartPointer.h"

#include <cstdlib>
#include <ctime>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
typedef int socklen_t;
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/select.h>
#include <sys/socket.h>
#endif
#include <errno.h>

extern int errno;

#include <vector>
#include <algorithm>

using namespace std;
using namespace Lib;

int ConnectRandomlyCommand::_nrand(int n){
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

int ConnectRandomlyCommand::_getRandomNum(int start,int length){
    if(start < 0)
        start = 0;
    else if(start > RAND_MAX)
        start = RAND_MAX;

    if(RAND_MAX - start < length)
        length = RAND_MAX - start;

    return _nrand(length) + start;
}

ClientSocket* ConnectRandomlyCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(TRY_TIME))
        return NULL;

    if(data.getInt(TYPE) != CONNECT_RANDOMLY)
        return NULL;

    ip_type destiny_ip = data.getString(DESTINY_IP);

    int try_time = data.getInt(TRY_TIME);

    if(try_time < 2)
        return NULL;

    fd_set set;
    FD_ZERO(&set);
    int maxfd = -1;
    int fd;

    vector<SmartPointer<ReuseClientSocket>> sockets(try_time);
    for(int i=0;i<try_time;++i){				// 创建 try_time 个 client socket 绑定相同的源端口，并将每个 socket 添加到 fd_set 中
        sockets[i].reset(new ReuseClientSocket());

        if(!sockets[i]->setNonBlock() || !sockets[i]->bind(ip,port))
            return NULL;

        fd = sockets[i]->_getfd();
        FD_SET(fd,&set);
        if(maxfd < fd)
            maxfd = fd;
    }

    sleep(CONNECT_SLEEP_TIME);

    vector<port_type> ports;
    port_type random_port;

    srand(time(NULL));

    for(int count=0;count < try_time;++count){
        do{
            random_port = _getRandomNum(1024,64512);
        }while(find(ports.begin(),ports.end(),random_port) != ports.end());
        ports.push_back(random_port);

        if(sockets[count]->connect(destiny_ip.c_str(),random_port)){	// 直接连接成功，不用后面的 select，直接返回该 socket
            sockets[count]->setNonBlock(false);
            int fd = sockets[count]->_getfd();
            sockets[count]->_invalid();
            return new ReuseClientSocket(fd);
        }else if(errno != EINPROGRESS){					// 若 errno == EINPROGRESS ，表示正在后台进行连接，属于正常情况，会在后面 select 中处理
            FD_CLR(sockets[count]->_getfd(),&set);		// 否则表示确定连接失败，将该 socket 从 fd_set 中删除，不用等它了
        }
    }

    // 连接成功，套接字可写
    // 连接失败，套接字可写且可读
    // 通过判断可写集合，再进一步通过 getsockopt 取得错误码，若连接错误，错误码不为0,否则表示连接成功
    struct timeval t = SELECT_WAIT_TIME;
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
                ::getsockopt(sockets[i].get()->_getfd(),SOL_SOCKET,SO_ERROR,(char*)&error,&len);

                if(error == 0){
                    sockets[i]->setNonBlock(false);			// 恢复阻塞属性

                    int fd = sockets[i].get()->_getfd();
                    sockets[i]->_invalid();

                    return new ReuseClientSocket(fd);
                }
            }
        }
        return NULL;
    }
}
