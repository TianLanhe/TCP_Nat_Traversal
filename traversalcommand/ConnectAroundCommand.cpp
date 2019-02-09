#include "ConnectAroundCommand.h"
#include "../socket/ReuseClientSocket.h"
#include "../include/SmartPointer.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
typedef int socklen_t;
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/select.h>
#include <sys/socket.h>
#endif
#include <errno.h>

#include <vector>

extern int errno;

using namespace Lib;
using namespace std;

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

	// 开启若干个套接字同时非阻塞连接，然后调用 select 等待连接成功
	// 不要用原来的一个套接字逐个连接，一次失败的连接最少得几秒，而对等方 Listen 的时间只有几秒钟，采用阻塞 connect 来不及在短时间内尝试多个连接

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
	// 开始连接，一口气发出 try_time 个连接，然后调用 select 阻塞等待连接结果
    for(int count=0;count < try_time && count * delta <= MAX_PORT - destiny_port;++count){
        if(sockets[count]->connect(destiny_ip.c_str(),destiny_port + count * delta)){	// 直接连接成功，不用后面的 select，直接返回该 socket
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
