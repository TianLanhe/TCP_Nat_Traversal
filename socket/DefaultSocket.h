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
	~DefaultSocket() { if (isOpen()) close(); }

	bool open();
	bool close();

	bool isOpen() { return m_socket != -1; }

	bool bind(port_type, const std::string&);	// 客户端服务器均可以绑定地址，不过客户端可以由系统分配不用显式调用
	bool isBound() { return m_port != -1 && !m_addr.empty(); }

	std::string read(int);
	size_t write(const char*);

	port_type getPort() { return m_port; }
	std::string getAddr() { return m_addr; }

	void updateAddrAndPort();
	void invalid();

public:
	int _socket() { return m_socket; }
	port_type _port() { return m_port; }
	std::string& _addr() { return m_addr; }

protected:
	std::string m_addr;
	port_type m_port;

	int m_socket;		// 用 -1 表示是否已经打开过

};

LIB_END

#endif // !DEFAULT_SOCKET_H
