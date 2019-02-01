#include "include/socket/SocketSelector.h"
#include "include/socket/Socket.h"
#include "include/Exception.h"
#include "DefaultClientSocket.h"
#include "DefaultServerSocket.h"

#include <cmath>
#include <sys/select.h>
#include <errno.h>

using namespace std;
using namespace Lib;

bool SocketSelector::add(Socket* socket){
    CHECK_PARAMETER_EXCEPTION(socket);

    for(vector<Socket*>::size_type i=0;i<m_sockets.size();++i)
        if(m_sockets[i] == socket)
            return false;

    m_sockets.push_back(socket);
    return true;
}

bool SocketSelector::remove(Socket* socket){
    for(vector<Socket*>::iterator it=m_sockets.begin();it != m_sockets.end();++it){
        if(*it == socket){
            m_sockets.erase(it);
            return true;
        }
    }

    return false;
}

bool SocketSelector::has(Socket* socket){
    for(vector<Socket*>::size_type i=0;i<m_sockets.size();++i)
        if(m_sockets[i] == socket)
            return true;

    return false;
}

int SocketSelector::_getfd(Socket *socket){
    int fd;
    DefaultServerSocket *server_socket;
    DefaultClientSocket *client_socket;

    if(server_socket = dynamic_cast<DefaultServerSocket*>(socket)){
        fd = server_socket->_getfd();
    }else if(client_socket = dynamic_cast<DefaultClientSocket*>(socket)){
        fd = client_socket->_getfd();
    }else{
        THROW_EXCEPTION(ErrorStateException,"the socket is neither DefaultClientSocket nor DefaultServerSocket");
    }

    return fd;
}

vector<Socket*> SocketSelector::select(double timeout){
    struct timeval to;
    struct timeval *ptrto;

    if(fabs(timeout) < 1e-5){
        ptrto = NULL;
    }else{
        to.tv_sec = time_t(timeout);
        to.tv_usec = long(timeout*1000000)%1000000;
        ptrto = &to;
    }

    vector<Socket*> retVec;

    while(1){
        int fd,maxfd;
        fd_set set;
        FD_ZERO(&set);

        for(vector<Socket*>::size_type i=0;i<m_sockets.size();++i){
            fd = _getfd(m_sockets[i]);

            if(maxfd < fd)
                maxfd = fd;

            FD_SET(fd,&set);
        }

        // 模板方法模式，子类实现
        int ret = _select(maxfd,&set,ptrto);

        if(ret == -1 && errno != EINTR || ret == 0){
            break;
        }else if(ret > 0){
            for(vector<Socket*>::size_type i=0;i<m_sockets.size();++i){
                fd = _getfd(m_sockets[i]);

                if(FD_ISSET(fd,&set)){
                    retVec.push_back(m_sockets[i]);

                    --ret;
                }
            }

            if(ret != 0)
                THROW_EXCEPTION(ErrorStateException,"return value of select is not equal to the fd which is ready");

            break;
        }
    }

    return retVec;
}
