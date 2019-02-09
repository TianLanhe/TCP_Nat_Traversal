#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "Socket.h"

LIB_BEGIN

class ClientSocket : public Socket
{
public:	// Window 下 connect 设置超时参数无效
    virtual bool connect(const ip_type& addr, port_type port, size_t trytime = 1,double timeout = 2.0) { return connect(addr.c_str(), port,trytime,timeout); }
    virtual bool connect(const char*, port_type, size_t trytime = 1,double timeout = 2.0) = 0;
	virtual bool isConnected() const = 0;

    virtual bool setNonBlock(bool flag = true) = 0;

    virtual ip_type getPeerAddr() const = 0;
    virtual port_type getPeerPort() const = 0;	// 这里设计成ClientSocket才能获取对方的信息
};

LIB_END

#endif
