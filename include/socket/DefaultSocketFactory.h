#ifndef DEFAULT_SOCKET_FACTORY_H
#define DEFAULT_SOCKET_FACTORY_H

#include "../Object.h"

LIB_BEGIN

class ClientSocket;
class ServerSocket;

class DefaultSocketFactory : public Object {
public:
	ServerSocket* GetDefaultServerSocket();
	ClientSocket* GetDefaultClientSocket();
	static DefaultSocketFactory* GetInstance();

private:
	DefaultSocketFactory() { }
	static DefaultSocketFactory* factory;
};

LIB_END

#endif
