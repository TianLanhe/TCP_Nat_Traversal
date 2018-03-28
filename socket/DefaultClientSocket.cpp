#include "DefaultClientSocket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>

using namespace std;
using namespace Lib;

DefaultClientSocket::DefaultClientSocket(int socket) :m_socket(socket)
{
	if (m_socket._socket() != -1)
		m_bHasConnect = true;
}

DefaultClientSocket::DefaultClientSocket(int socket,const string& addr,port_type port) :m_socket(socket,addr,port)
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

bool DefaultClientSocket::connect(const char* addr, port_type port, size_t time)
{
    CHECK_PARAMETER_EXCEPTION(time != 0 && addr != NULL);
	CHECK_OPERATION_EXCEPTION(!m_bHasConnect && isOpen());

	struct sockaddr_in server_addr;
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_port = htons(port);
	server_addr.sin_family = AF_INET;

    size_t max_try_time = time < _getMaxTryTime() ? time : _getMaxTryTime();
    size_t try_time = 0;			//如果不成功每隔一秒连接一次
    while (try_time < max_try_time && ::connect(m_socket._socket(), (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        ++try_time;

        if(try_time != max_try_time)
            usleep(_getSleepTime());		// Review：有些套接字实现若connect失败则以后都会失败，需要关闭后重新打开套接字
    }

    if(try_time == max_try_time)
        return false;

	m_bHasConnect = true;

	if (!isBound())
		m_socket.updateAddrAndPort();

    CHECK_STATE_EXCEPTION(m_socket._port() != (unsigned short)(-1) && m_socket._addr() != "");

	return true;
}

bool DefaultClientSocket::setNonBlock(bool flag){
    int f = fcntl(m_socket._socket(),F_GETFL,0);
    f = ( flag ? f | O_NONBLOCK : f & ~O_NONBLOCK);
    return fcntl(m_socket._socket(),F_SETFL,f) == 0;
}

string DefaultClientSocket::read(int read_byte)
{
	CHECK_OPERATION_EXCEPTION(m_bHasConnect);

	return m_socket.read(read_byte);
}

size_t DefaultClientSocket::write(const char* content)
{
	CHECK_OPERATION_EXCEPTION(m_bHasConnect);

	return m_socket.write(content);
}

typename DefaultClientSocket::port_type DefaultClientSocket::getPeerPort() const {
	CHECK_OPERATION_EXCEPTION(isOpen() && isBound());

	struct sockaddr_in server_addr;
	socklen_t len = sizeof(server_addr);
	getpeername(m_socket._socket(), (struct sockaddr*)&server_addr, &len);

	return ntohs(server_addr.sin_port);
}

std::string DefaultClientSocket::getPeerAddr() const {
	CHECK_OPERATION_EXCEPTION(isOpen() && isBound());

	struct sockaddr_in server_addr;
	socklen_t len = sizeof(server_addr);
	getpeername(m_socket._socket(), (struct sockaddr*)&server_addr, &len);

	return inet_ntoa(server_addr.sin_addr);
}
