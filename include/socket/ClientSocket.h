#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "Socket.h"

LIB_BEGIN

class ClientSocket : public Socket
{
public:
	virtual bool connect(const std::string& addr, port_type port) { return connect(addr.c_str(), port); }
	virtual bool connect(const char*, port_type) = 0;
	virtual bool isConnected() = 0;

	virtual std::string getPeerAddr() = 0;	// Review：服务器accept时返回ClientServer，这时候ClientServer的PeerInfo和自己的info都是一样的
	virtual port_type getPeerPort() = 0;	// 这里设计成ClientServer才能获取对方的信息
};

LIB_END

#endif