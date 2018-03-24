#include "DefaultServerSocket.h"
#include "DefaultClientSocket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

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
	CHECK_OPERATION_EXCEPTION(m_bIsListen);

	return m_socket.read(read_byte);
}

size_t DefaultServerSocket::write(const char* content)
{
	CHECK_OPERATION_EXCEPTION(m_bIsListen);

	return m_socket.write(content);;
}

bool DefaultServerSocket::listen(int num)
{
	CHECK_OPERATION_EXCEPTION(!m_bIsListen);

	int ret = ::listen(m_socket._socket(), num);	//设置最大监听数并监听

	if (ret == 0)
		m_bIsListen = true;

	return (ret == 0);
}

ClientSocket* DefaultServerSocket::accept()
{
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);

    int client_socket = ::accept(m_socket._socket(), (struct sockaddr*)&cli_addr, &len);

    return (client_socket == -1 ? NULL : new DefaultClientSocket(client_socket,inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port)));
}
