#include "DefaultClientSocket.h"

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
typedef int socklen_t;
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <cstring>
#include <cstdlib>

using namespace std;
using namespace Lib;

DefaultClientSocket::DefaultClientSocket(int socket) :m_socket(socket)
{
	if (m_socket._socket() != -1)
		m_bHasConnect = true;
}

DefaultClientSocket::DefaultClientSocket(int socket,const ip_type& addr,port_type port) :m_socket(socket,addr,port)
{
    if (m_socket._socket() != -1)
        m_bHasConnect = true;
}

bool DefaultClientSocket::close()
{
    bool ret = m_socket.close();

    if (ret)
		m_bHasConnect = false;

    return ret;
}

bool DefaultClientSocket::connect(const char* addr, port_type port, size_t trytime, double timeout)
{
    CHECK_PARAMETER_EXCEPTION(trytime != 0 && addr != NULL);
    CHECK_PARAMETER_EXCEPTION(timeout > 0);
    CHECK_OPERATION_EXCEPTION(!m_bHasConnect && isOpen());

	struct sockaddr_in server_addr;
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_port = htons(port);
	server_addr.sin_family = AF_INET;

    // 获取之前的设置，一会用于恢复
    struct timeval pre;
    socklen_t len;
    ::getsockopt(m_socket._socket(),SOL_SOCKET,SO_SNDTIMEO,(char*)&pre,&len);

    // 设置连接超时时间
    struct timeval timeo = {time_t(timeout),long(timeout*1000000)%1000000};
    ::setsockopt(m_socket._socket(),SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeo,sizeof(timeo));

    size_t max_try_time = trytime < _getMaxTryTime() ? trytime : _getMaxTryTime();
    size_t try_time = 0;			//如果不成功每隔一秒连接一次
    while (try_time < max_try_time && ::connect(m_socket._socket(), (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        ++try_time;

		if (try_time != max_try_time)
#if defined(_WIN32) || defined(_WIN64)
			Sleep(_getSleepTime() / 1000);
#elif defined(__linux__) || defined(__APPLE__)
            usleep(_getSleepTime());		// Review：有些套接字实现若connect失败则以后都会失败，需要关闭后重新打开套接字
#endif
	}

    // 恢复之前的连接超时时间
    ::setsockopt(m_socket._socket(),SOL_SOCKET,SO_SNDTIMEO,(char*)&pre,sizeof(pre));

    if(try_time == max_try_time)
        return false;

	m_bHasConnect = true;

	if (!isBound())
		m_socket.updateAddrAndPort();

    CHECK_STATE_EXCEPTION(m_socket._port() != (unsigned short)(-1) && m_socket._addr() != "");

	return true;
}

bool DefaultClientSocket::setNonBlock(bool flag){
#if defined(_WIN32) || defined(_WIN64)
	return true;
#elif defined(__linux__) || defined(__APPLE__)
    int f = fcntl(m_socket._socket(),F_GETFL,0);

    if(f < 0){
        return false;
    }

    f = ( flag ? f | O_NONBLOCK : f & ~O_NONBLOCK);
    return fcntl(m_socket._socket(),F_SETFL,f) == 0;
#endif
}

string DefaultClientSocket::read(int read_byte)
{
    CHECK_PARAMETER_EXCEPTION(read_byte > 0);
	CHECK_OPERATION_EXCEPTION(m_bHasConnect);

	return m_socket.read(read_byte);
}

size_t DefaultClientSocket::write(const char* content)
{
    CHECK_PARAMETER_EXCEPTION(content);
	CHECK_OPERATION_EXCEPTION(m_bHasConnect);

	return m_socket.write(content);
}

size_t DefaultClientSocket::read(char* buffer,int read_byte)
{
    CHECK_PARAMETER_EXCEPTION(buffer && read_byte > 0);
    CHECK_OPERATION_EXCEPTION(m_bHasConnect);

    return m_socket.read(buffer,read_byte);
}

size_t DefaultClientSocket::write(const char* buffer,size_t size)
{
    CHECK_PARAMETER_EXCEPTION(buffer && size > 0);
    CHECK_OPERATION_EXCEPTION(m_bHasConnect);

    return m_socket.write(buffer,size);
}

typename DefaultClientSocket::port_type DefaultClientSocket::getPeerPort() const {
    CHECK_OPERATION_EXCEPTION(isOpen() && isBound() && isConnected());

	struct sockaddr_in server_addr;
	socklen_t len = sizeof(server_addr);
	getpeername(m_socket._socket(), (struct sockaddr*)&server_addr, &len);

	return ntohs(server_addr.sin_port);
}

typename DefaultClientSocket::ip_type DefaultClientSocket::getPeerAddr() const {
    CHECK_OPERATION_EXCEPTION(isOpen() && isBound() && isConnected());

	struct sockaddr_in server_addr;
	socklen_t len = sizeof(server_addr);
	getpeername(m_socket._socket(), (struct sockaddr*)&server_addr, &len);

	return inet_ntoa(server_addr.sin_addr);
}
