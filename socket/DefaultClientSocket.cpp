#include "DefaultClientSocket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

using namespace std;
using namespace Lib;

DefaultClientSocket::DefaultClientSocket(int socket) :m_socket(socket)
{
	if (m_socket._socket() != -1)
		m_bHasConnect = true;
}

bool DefaultClientSocket::close()
{
	int ret = m_socket.close();

	if (ret == 0)
		m_bHasConnect = false;

	return ret == 0;
}

bool DefaultClientSocket::connect(const char* addr, port_type port)
{
	CHECK_OPERATION_EXCEPTION(!m_bHasConnect && isOpen());

	struct sockaddr_in server_addr;
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_port = htons(port);
	server_addr.sin_family = AF_INET;

	int try_time = 0;			//如果不成功每隔一秒连接一次，最多10次
	while (try_time++ < _getMaxTryTime() && ::connect(m_socket._socket(), (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		sleep(1);

	if (try_time == 10)	// 连接失败
		return false;

	m_bHasConnect = true;

	if (!isBound())
		m_socket.updateAddrAndPort();

	CHECK_STATE_EXCEPTION(m_socket._port() != -1 && m_socket._addr() != "");

	return true;
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
