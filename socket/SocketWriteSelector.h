#ifndef SOCKET_WRITE_SELECTOR_H
#define SOCKET_WRITE_SELECTOR_H

#include "include/socket/SocketSelector.h"

LIB_BEGIN

class SocketWriteSelector : public SocketSelector
{
protected:
    virtual int _select(int maxfd,fd_set* ptrSet,struct timeval* ptrTo);
};

LIB_END

#endif // !SOCKET_WRITE_SELECTOR_H
