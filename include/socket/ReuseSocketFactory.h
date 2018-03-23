#ifndef REUSE_SOCKET_FACTORY_H
#define REUSE_SOCKET_FACTORY_H

#include "../Object.h"

LIB_BEGIN

class ClientSocket;
class ServerSocket;

class ReuseSocketFactory : public Object {
public:
    ServerSocket* GetServerSocket();
    ClientSocket* GetClientSocket();
	static ReuseSocketFactory* GetInstance();

private:
	ReuseSocketFactory() { }
	static ReuseSocketFactory* factory;
};

LIB_END

#endif
