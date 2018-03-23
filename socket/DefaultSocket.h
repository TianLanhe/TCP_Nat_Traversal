#ifndef DEFAULT_SOCKET_H
#define DEFAULT_SOCKET_H

#include "../include/Object.h"

#include <string>

LIB_BEGIN

class DefaultSocket : public Object{

public:
	typedef unsigned short port_type;

public:
	DefaultSocket() :m_port(-1), m_socket(-1){ open(); }
	DefaultSocket(int);
    DefaultSocket(int,const std::string&,port_type);
	~DefaultSocket() { if (isOpen()) close(); }

	bool open();
	bool close();

	bool isOpen() const { return m_socket != -1; }

    bool bind(const std::string&, port_type);	// 客户端服务器均可以绑定地址，不过客户端可以由系统分配不用显式调用
    bool isBound() const { return m_port != (unsigned short)(-1) && !m_addr.empty(); }

	std::string read(int);
	size_t write(const char*);

	port_type getPort() const { return m_port; }
	std::string getAddr() const { return m_addr; }

	void updateAddrAndPort();
	void invalid();

public:
	int _socket() const { return m_socket; }
	port_type _port() const { return m_port; }
	std::string _addr() const { return m_addr; }

protected:
	std::string m_addr;
	port_type m_port;

	int m_socket;		// 用 -1 表示是否已经打开过

};

LIB_END

#endif // !DEFAULT_SOCKET_H
