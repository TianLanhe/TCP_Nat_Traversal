#ifndef REUSE_SERVER_SOCKET_H
#define REUSE_SERVER_SOCKET_H

#include "DefaultServerSocket.h"

LIB_BEGIN

class ReuseServerSocket : public DefaultServerSocket {

public:
	ReuseServerSocket() { if (isOpen()) setReuse(); }

	virtual bool open() {
		return DefaultServerSocket::open() && setReuse();
	}

protected:
	bool setReuse();

};

LIB_END

#endif // !REUSE_SERVER_SOCKET_H
