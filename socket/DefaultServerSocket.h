#ifndef DEFAULT_SERVER_SOCKET_H
#define DEFAULT_SERVER_SOCKET_H

#include "../include/socket/ServerSocket.h"
#include "DefaultSocket.h"

LIB_BEGIN

#define DEFAULT_READ_BYTE 512

class DefaultServerSocket : public ServerSocket {
public:
	DefaultServerSocket() :m_bIsListen(false) { }

	bool open() { return m_socket.open(); }
	bool isOpen() { return m_socket.isOpen(); }
	bool close();

	bool bind(port_type port, const std::string& addr = "") { return m_socket.bind(port, addr); }
	bool isBound() { return m_socket.isBound(); }

	std::string read(int read_bype = DEFAULT_READ_BYTE);		// 默认读取 512 个字节
	size_t write(const char*);

	bool listen(int);
	bool isListen() { return m_bIsListen; }

	ClientSocket* accept();

	port_type getPort() { return m_socket._port(); }
	std::string getAddr() { return m_socket._addr(); }

protected:
	explicit DefaultServerSocket(int socket);

protected:
	DefaultSocket m_socket;

	bool m_bIsListen;
};

LIB_END

#endif
