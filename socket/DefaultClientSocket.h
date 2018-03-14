#ifndef DEFAULT_CLIENT_SOCKET_H
#define DEFAULT_CLIENT_SOCKET_H

#include "../include/socket/ClientSocket.h"
#include "DefaultSocket.h"

LIB_BEGIN

#define DEFAULT_READ_BYTE 512

class DefaultClientSocket : public ClientSocket
{

public:
	DefaultClientSocket() :m_bHasConnect(false) { }
	explicit DefaultClientSocket(int socket);		// 本来想写成protected的，但是这样得申明DefaultServerSocket是它的友元，想想还是这么处理吧
    DefaultClientSocket(int socket,const std::string& addr,port_type port);

	bool open() { return m_socket.open(); }
	bool close();
	bool isOpen() { return m_socket.isOpen(); }

	bool bind(port_type port, const std::string& addr) { return m_socket.bind(port, addr); }
	bool isBound() { return m_socket.isBound(); }

	std::string read(int read_bype = DEFAULT_READ_BYTE);		// 默认读取 512 个字节
	size_t write(const char*);

	bool connect(const char*, port_type);
	bool isConnected() { return m_bHasConnect; }

	port_type getPort() { return m_socket._port(); }
	std::string getAddr() { return m_socket._addr(); }

protected:

	int _getMaxTryTime() { return 10; }

protected:
	DefaultSocket m_socket;

	bool m_bHasConnect;
};

LIB_END

#endif
