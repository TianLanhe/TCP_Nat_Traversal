#include "include/socket/SocketSelectorFactory.h"
#include "include/socket/SocketSelector.h"
#include "SocketReadSelector.h"
#include "SocketWriteSelector.h"

using namespace Lib;

SocketSelectorFactory* SocketSelectorFactory::factory = NULL;

SocketSelector* SocketSelectorFactory::GetReadSelector(){
    SocketSelector *ret = new SocketReadSelector();
	CHECK_NO_MEMORY_EXCEPTION(ret);
	return ret;
}

SocketSelector* SocketSelectorFactory::GetWriteSelector(){
    SocketSelector *ret = new SocketWriteSelector();
	CHECK_NO_MEMORY_EXCEPTION(ret);
	return ret;
}

SocketSelectorFactory* SocketSelectorFactory::GetInstance(){
	if(!factory){
        factory = new SocketSelectorFactory();
		CHECK_NO_MEMORY_EXCEPTION(factory);
	}
	return factory;
}
