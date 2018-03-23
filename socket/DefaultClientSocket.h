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
	explicit DefaultClientSocket(int socket);		// ������д��protected�ģ���������������DefaultServerSocket��������Ԫ�����뻹����ô�����
    DefaultClientSocket(int socket,const std::string& addr,port_type port);

	virtual bool open() { return m_socket.open(); }
	virtual bool close();
	virtual bool isOpen() const { return m_socket.isOpen(); }

    virtual bool bind(port_type port) { return m_socket.bind("",port); }
    virtual bool bind(const std::string& addr, port_type port) { return m_socket.bind(addr,port); }
	virtual bool isBound() const { return m_socket.isBound(); }

	virtual std::string read(int read_bype = DEFAULT_READ_BYTE);		// Ĭ�϶�ȡ 512 ���ֽ�
	virtual size_t write(const char*);

    virtual bool connect(const char*, port_type, size_t time = (size_t)(-1));
	virtual bool isConnected() const { return m_bHasConnect; }

	virtual port_type getPort() const { return m_socket._port(); }
	virtual std::string getAddr() const { return m_socket._addr(); }

	virtual port_type getPeerPort() const;
	virtual std::string getPeerAddr() const;

protected:

    size_t _getMaxTryTime() const { return 10; }

protected:
	DefaultSocket m_socket;

	bool m_bHasConnect;
};

LIB_END

#endif
