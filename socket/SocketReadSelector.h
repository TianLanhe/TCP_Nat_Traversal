#ifndef SOCKET_READ_SELECTOR_H
#define SOCKET_READ_SELECTOR_H

#include "include/socket/SocketSelector.h"

LIB_BEGIN

class SocketReadSelector : public SocketSelector
{
protected:
    virtual int _select(int maxfd,fd_set* ptrSet,struct timeval* ptrTo);
};

LIB_END

#endif // !SOCKET_READ_SELECTOR_H
