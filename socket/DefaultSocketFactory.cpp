#include "../include/socket/DefaultSocketFactory.h"
#include "DefaultServerSocket.h"
#include "DefaultClientSocket.h"

using namespace Lib;

DefaultSocketFactory* DefaultSocketFactory::factory = NULL;

ServerSocket* DefaultSocketFactory::GetDefaultServerSocket(){
	return new DefaultServerSocket();
}

ClientSocket* DefaultSocketFactory::GetDefaultClientSocket(){
	return new DefaultClientSocket();
}

DefaultSocketFactory* DefaultSocketFactory::GetInstance(){
	if(!factory)
		factory = new DefaultSocketFactory();
	return factory;
}
