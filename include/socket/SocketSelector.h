#ifndef SOCKET_SELECTOR_H
#define SOCKET_SELECTOR_H

#include "include/Object.h"

#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/select.h>
#endif

LIB_BEGIN

class Socket;

class SocketSelector : public Object
{
public:
    typedef std::vector<Socket*> SocketVector;

public:

    bool add(Socket*);
    bool remove(Socket*);
    bool has(Socket*);
    void clear() { m_sockets.clear(); }

    virtual SocketVector select(double timeout = 0.0);

    size_t size() const { return m_sockets.size(); }
    bool empty() const { return m_sockets.empty(); }

protected:
    virtual int _select(int maxfd,fd_set* ptrSet,struct timeval* ptrTo) = 0;

    int _getfd(Socket*);

protected:
    std::vector<Socket*> m_sockets;
};

LIB_END

#endif // !SOCKET_SELECTOR_H
