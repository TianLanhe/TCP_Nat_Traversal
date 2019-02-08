#include "DefaultServerSocket.h"
#include "DefaultClientSocket.h"

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
typedef int socklen_t;
#elif defined(linux) || defined(__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <cstring>
#include <cmath>

using namespace std;
using namespace Lib;

DefaultServerSocket::DefaultServerSocket(int socket) :m_socket(socket)
{
	if (socket != -1)
		m_bIsListen = true;
}

bool DefaultServerSocket::close()
{
    bool ret = m_socket.close();

    if (ret)
		m_bIsListen = false;

    return ret;
}

string DefaultServerSocket::read(int read_byte)
{
    CHECK_PARAMETER_EXCEPTION(read_byte > 0);
	CHECK_OPERATION_EXCEPTION(m_bIsListen);

	return m_socket.read(read_byte);
}

size_t DefaultServerSocket::write(const char* content)
{
    CHECK_PARAMETER_EXCEPTION(content);
	CHECK_OPERATION_EXCEPTION(m_bIsListen);

	return m_socket.write(content);;
}

size_t DefaultServerSocket::read(char* buffer, int read_byte)
{
    CHECK_PARAMETER_EXCEPTION(buffer && read_byte > 0);
    CHECK_OPERATION_EXCEPTION(m_bIsListen);

    return m_socket.read(buffer,read_byte);
}

size_t DefaultServerSocket::write(const char* buffer, size_t size)
{
    CHECK_PARAMETER_EXCEPTION(buffer && size > 0);
    CHECK_OPERATION_EXCEPTION(m_bIsListen);

    return m_socket.write(buffer,size);
}

bool DefaultServerSocket::listen(int num)
{
    CHECK_OPERATION_EXCEPTION(!m_bIsListen && isBound());

	int ret = ::listen(m_socket._socket(), num);	//设置最大监听数并监听

	if (ret == 0)
		m_bIsListen = true;

	return (ret == 0);
}

ClientSocket* DefaultServerSocket::accept(double timeout)
{
    CHECK_OPERATION_EXCEPTION(isListen());

    struct timeval pre;
    if(fabs(timeout+1.0) > 1e-5){   // timeout != -1
        // 获取之前的设置，一会用于恢复
        socklen_t len;
        ::getsockopt(m_socket._socket(),SOL_SOCKET,SO_RCVTIMEO,(char*)&pre,&len);

        // 设置连接超时时间
        struct timeval timeo = {time_t(timeout),long(timeout*1000000)%1000000};
        ::setsockopt(m_socket._socket(),SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeo,sizeof(timeo));
    }

    int client_socket = ::accept(m_socket._socket(), NULL, NULL);

    if(fabs(timeout+1.0) > 1e-5){   // timeout != -1
        ::setsockopt(m_socket._socket(),SOL_SOCKET,SO_RCVTIMEO, (const char*)&pre,sizeof(pre));
    }

    return (client_socket == -1 ? NULL : new DefaultClientSocket(client_socket,m_socket._addr(),m_socket._port()));
}
