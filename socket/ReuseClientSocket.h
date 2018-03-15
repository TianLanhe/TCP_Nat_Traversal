#ifndef REUSE_CLIENT_SOCKET_H
#define REUSE_CLIENT_SOCKET_H

#include "DefaultClientSocket.h"

LIB_BEGIN

class ReuseClientSocket : public DefaultClientSocket {

public:
	ReuseClientSocket() { if (isOpen()) setReuse(); }
	
	virtual bool open() {
		return DefaultClientSocket::open() && setReuse();
	}

protected:
	bool setReuse();

};

LIB_END

#endif // !REUSE_CLIENT_SOCKET_H
