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

ClientSocket* DefaultServerSocket::accept()
{
    CHECK_OPERATION_EXCEPTION(isListen());

    int client_socket = ::accept(m_socket._socket(), NULL, NULL);

    return (client_socket == -1 ? NULL : new DefaultClientSocket(client_socket,m_socket._addr(),m_socket._port()));
}
