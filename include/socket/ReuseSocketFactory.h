#ifndef REUSE_SOCKET_FACTORY_H
#define REUSE_SOCKET_FACTORY_H

#include "../Object.h"

LIB_BEGIN

class ClientSocket;
class ServerSocket;

class ReuseSocketFactory : public Object {
public:
	ServerSocket* GetReuseServerSocket();
	ClientSocket* GetReuseClientSocket();
	static ReuseSocketFactory* GetInstance();

private:
	ReuseSocketFactory() { }
	static ReuseSocketFactory* factory;
};

LIB_END

#endif
