#ifndef SOCKET_SELECTOR_FACTORY_H
#define SOCKET_SELECTOR_FACTORY_H

#include "../Object.h"

LIB_BEGIN

class SocketSelector;

class SocketSelectorFactory : public Object {
public:
    SocketSelector* GetWriteSelector();
    SocketSelector* GetReadSelector();
    static SocketSelectorFactory* GetInstance();

private:
    SocketSelectorFactory() { }
    static SocketSelectorFactory* factory;
};

LIB_END

#endif
