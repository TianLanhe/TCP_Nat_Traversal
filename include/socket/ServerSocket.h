#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "Socket.h"

LIB_BEGIN

class ClientSocket;

class ServerSocket : public Socket
{
public:
	virtual bool listen(int) = 0;
	virtual bool isListen() = 0;

	virtual bool bind(port_type,const std::string& addr = "") = 0;

	virtual ClientSocket* accept() = 0;
};

LIB_END

#endif
