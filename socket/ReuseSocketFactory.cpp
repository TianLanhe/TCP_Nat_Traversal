#include "../include/socket/ReuseSocketFactory.h"
#include "ReuseServerSocket.h"
#include "ReuseClientSocket.h"

using namespace Lib;

ReuseSocketFactory* ReuseSocketFactory::factory = NULL;

ServerSocket* ReuseSocketFactory::GetReuseServerSocket() {
	ServerSocket *ret = new ReuseServerSocket();
	CHECK_NO_MEMORY_EXCEPTION(ret);
	return ret;
}

ClientSocket* ReuseSocketFactory::GetReuseClientSocket() {
	ClientSocket *ret = new ReuseClientSocket();
	CHECK_NO_MEMORY_EXCEPTION(ret);
	return ret;
}

ReuseSocketFactory* ReuseSocketFactory::GetInstance() {
	if (!factory) {
		factory = new ReuseSocketFactory();
		CHECK_NO_MEMORY_EXCEPTION(factory);
	}
	return factory;
}
