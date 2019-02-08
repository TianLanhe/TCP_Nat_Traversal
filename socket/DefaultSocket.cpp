#include "DefaultSocket.h"

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
typedef int socklen_t;
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <cstring>
#include <cstdlib>

using namespace std;
using namespace Lib;

DefaultSocket::DefaultSocket(int socket)
{
	invalid();

	m_socket = socket;
	if (socket != -1) {
		// 默认该socket已经绑定到某一端口和地址，并已与服务器socket链接
		updateAddrAndPort();
	}
}

DefaultSocket::DefaultSocket(int socket, const ip_type& addr, port_type port) {
	invalid();

	m_socket = socket;
	if (socket != -1) {
		m_port = port;
		m_addr = addr;
	}
}

bool DefaultSocket::open()
{
	CHECK_OPERATION_EXCEPTION(!isOpen());

	// 前置条件：关闭时已经是 invalid 状态
	m_socket = ::socket(AF_INET, SOCK_STREAM, 0);	//创建客户端套接字

	return m_socket != -1;
}

bool DefaultSocket::close()
{
	CHECK_OPERATION_EXCEPTION(isOpen());

#if defined(_WIN32) || defined(_WIN64)
	int r = ::closesocket(m_socket);
#elif defined(__linux__) || defined(__APPLE__)
	int r = ::close(m_socket);
#endif

	if (r == 0)		// 后置条件：成功关闭将状态置为 invalid
		invalid();

	return r == 0;
}

string DefaultSocket::read(int read_byte)
{
	CHECK_PARAMETER_EXCEPTION(read_byte > 0);
	CHECK_OPERATION_EXCEPTION(isOpen());

	string ret;

	char *content = (char*)malloc(sizeof(char)*(read_byte + 1));
	CHECK_NO_MEMORY_EXCEPTION(content);
	content[0] = '\0';

#if defined(_WIN32) || defined(_WIN64)
	size_t b = ::recv(m_socket, content, read_byte, 0);
#elif defined(__linux__) || defined(__APPLE__)
	size_t b = ::read(m_socket, content, read_byte);
#endif

	content[(b >= 0 ? b : 0)] = '\0';

	ret.append(content);

	free(content);

	return ret;
}

size_t DefaultSocket::write(const char* content)
{
	CHECK_PARAMETER_EXCEPTION(content);
	CHECK_OPERATION_EXCEPTION(isOpen());

#if defined(_WIN32) || defined(_WIN64)
	int write_byte = ::send(m_socket, content, strlen(content),0);
#elif defined(__linux__) || defined(__APPLE__)
	int write_byte = ::write(m_socket, content, strlen(content));
#endif

	return write_byte;
}

size_t DefaultSocket::read(char *buffer, int read_byte)
{
	CHECK_PARAMETER_EXCEPTION(buffer && read_byte > 0);
	CHECK_OPERATION_EXCEPTION(isOpen());

#if defined(_WIN32) || defined(_WIN64)
	return ::recv(m_socket, buffer, read_byte, 0);
#elif defined(__linux__) || defined(__APPLE__)
	return ::read(m_socket, buffer, read_byte);
#endif
}

size_t DefaultSocket::write(const char * buffer, size_t size)
{
	CHECK_PARAMETER_EXCEPTION(buffer && size > 0);
	CHECK_OPERATION_EXCEPTION(isOpen());

#if defined(_WIN32) || defined(_WIN64)
	return ::send(m_socket, buffer, size, 0);
#elif defined(__linux__) || defined(__APPLE__)
	return ::write(m_socket, buffer, size);
#endif
}

void DefaultSocket::invalid()
{
	m_socket = -1;
	m_port = -1;
	m_addr = "";
}

bool DefaultSocket::bind(const ip_type& addr, port_type port)
{
	CHECK_OPERATION_EXCEPTION(isOpen() && !isBound());

	struct sockaddr_in cli_addr;
	socklen_t len = sizeof(cli_addr);

	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = (addr.empty() ? htonl(INADDR_ANY) : inet_addr(addr.c_str()));
	cli_addr.sin_port = htons(port);

	int state = ::bind(m_socket, (struct sockaddr*)&cli_addr, len);

	if (state == -1)
		return false;

	m_addr = (addr.empty() ? inet_ntoa(cli_addr.sin_addr) : addr);
	m_port = port;

	return true;
}

void DefaultSocket::updateAddrAndPort()
{
	struct sockaddr_in cli_addr;
	socklen_t len = sizeof(cli_addr);
	getsockname(m_socket, (struct sockaddr*)&cli_addr, &len);

	m_addr = inet_ntoa(cli_addr.sin_addr);
	m_port = ntohs(cli_addr.sin_port);
}
