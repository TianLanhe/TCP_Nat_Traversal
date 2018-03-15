#ifndef DEFAULT_SERVER_SOCKET_H
#define DEFAULT_SERVER_SOCKET_H

#include "../include/socket/ServerSocket.h"
#include "DefaultSocket.h"

LIB_BEGIN

#define DEFAULT_READ_BYTE 512

class DefaultServerSocket : public ServerSocket {
public:
	DefaultServerSocket() :m_bIsListen(false) { }
    explicit DefaultServerSocket(int socket);

	virtual bool open() { return m_socket.open(); }
	virtual bool isOpen() { return m_socket.isOpen(); }
	virtual bool close();

	virtual bool bind(port_type port, const std::string& addr = "") { return m_socket.bind(port, addr); }
	virtual bool isBound() { return m_socket.isBound(); }

	virtual std::string read(int read_bype = DEFAULT_READ_BYTE);		// 默认读取 512 个字节
	virtual size_t write(const char*);

	virtual bool listen(int);
	virtual bool isListen() { return m_bIsListen; }

	virtual ClientSocket* accept();

	virtual port_type getPort() { return m_socket._port(); }
	virtual std::string getAddr() { return m_socket._addr(); }

protected:

protected:
	DefaultSocket m_socket;

	bool m_bIsListen;
};

LIB_END

#endif
