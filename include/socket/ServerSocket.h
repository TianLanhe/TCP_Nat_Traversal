#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "Socket.h"

LIB_BEGIN

#define DEFAULT_LISTEN_NUM 10
#define DEFAULT_ACCEPT_TIMEOUT 1.0

class ClientSocket;

class ServerSocket : public Socket
{
public:
	virtual bool listen(int) = 0;
	virtual bool isListen() const = 0;

    virtual ClientSocket* accept(double timeout = -1.0) = 0;// Window 下 accept 设置超时参数无效
};

LIB_END

#endif
