#include "DefaultSocket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
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

DefaultSocket::DefaultSocket(int socket,const string& addr,port_type port){
    invalid();

    m_socket = socket;
    if(socket != -1){
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

	int r = ::close(m_socket);

	if (r == 0)		// 后置条件：成功关闭将状态置为 invalid
		invalid();

	return r == 0;
}

string DefaultSocket::read(int read_byte)
{
	CHECK_OPERATION_EXCEPTION(isOpen());

	string ret;

	char *content = (char*)malloc(sizeof(char)*(read_byte + 1));
	CHECK_NO_MEMORY_EXCEPTION(content);
	content[0] = '\0';

    size_t b = ::read(m_socket, content, read_byte);
	content[b] = '\0';

	ret.append(content);

	return ret;
}

size_t DefaultSocket::write(const char* content)
{
	CHECK_OPERATION_EXCEPTION(isOpen() && content);

    int write_byte = ::write(m_socket, content, strlen(content));

    return write_byte;
}

void DefaultSocket::invalid() {
	m_socket = -1;
	m_port = -1;
	m_addr = "";
}

bool DefaultSocket::bind(port_type port, const std::string& addr) {
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

void DefaultSocket::updateAddrAndPort() {
	struct sockaddr_in cli_addr;
	socklen_t len = sizeof(cli_addr);
    getsockname(m_socket, (struct sockaddr*)&cli_addr, &len);

	m_addr = inet_ntoa(cli_addr.sin_addr);
    m_port = ntohs(cli_addr.sin_port);
}
