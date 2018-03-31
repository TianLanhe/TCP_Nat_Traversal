#ifndef DEFAULT_SERVER_SOCKET_H
#define DEFAULT_SERVER_SOCKET_H

#include "../include/socket/ServerSocket.h"
#include "DefaultSocket.h"

LIB_BEGIN

#define DEFAULT_READ_BYTE 512

class DefaultServerSocket : public ServerSocket {
public:
	DefaultServerSocket() :m_bIsListen(false) { }
    explicit DefaultServerSocket(int socket);	// Review : �Ƿ���Է���protected��

	virtual bool open() { return m_socket.open(); }
	virtual bool isOpen() const { return m_socket.isOpen(); }
	virtual bool close();

    virtual bool bind(port_type port) { return m_socket.bind("", port); }
    virtual bool bind(const std::string& addr, port_type port) { return m_socket.bind(addr, port); }
	virtual bool isBound() const { return m_socket.isBound(); }

	virtual std::string read(int read_bype = DEFAULT_READ_BYTE);		// Ĭ�϶�ȡ 512 ���ֽ�
	virtual size_t write(const char*);

	virtual bool listen(int);
	virtual bool isListen() const { return m_bIsListen; }

	virtual ClientSocket* accept();

	virtual port_type getPort() const { return m_socket._port(); }
	virtual std::string getAddr() const { return m_socket._addr(); }

public:
    int _getfd(){ return m_socket._socket(); }		// �ṩ�� NatTraversalServer �����⺯��

protected:
	DefaultSocket m_socket;

	bool m_bIsListen;
};

LIB_END

#endif
