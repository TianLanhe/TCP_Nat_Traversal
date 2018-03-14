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
};

LIB_END

#endif