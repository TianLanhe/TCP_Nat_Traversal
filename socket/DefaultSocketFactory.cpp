#include "../include/socket/DefaultSocketFactory.h"
#include "DefaultServerSocket.h"
#include "DefaultClientSocket.h"

using namespace Lib;

DefaultSocketFactory* DefaultSocketFactory::factory = NULL;

ServerSocket* DefaultSocketFactory::GetServerSocket(){
	ServerSocket *ret = new DefaultServerSocket();
	CHECK_NO_MEMORY_EXCEPTION(ret);
	return ret;
}

ClientSocket* DefaultSocketFactory::GetClientSocket(){
	ClientSocket *ret = new DefaultClientSocket();
	CHECK_NO_MEMORY_EXCEPTION(ret);
	return ret;
}

DefaultSocketFactory* DefaultSocketFactory::GetInstance(){
	if(!factory){
		factory = new DefaultSocketFactory();
		CHECK_NO_MEMORY_EXCEPTION(factory);
	}
	return factory;
}
