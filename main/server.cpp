#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../include/socket/DefaultSocketFactory.h"
#include "../include/socket/ServerSocket.h"

#include <iostream>
#include <string>

using namespace std;
using namespace Lib;

int main(int argc,char *argv[]){
	ServerSocket *server = DefaultSocketFactory::GetInstance()->GetDefaultServerSocket();

	ClientSocket *client;
	TransmissionProxy proxy;

	if(!server->bind(8888)){
		cout << "server bind 8888 error!" << endl;
		return 1;
	}else
		cout << "server has bound 8888 !"<< endl;

	if(!server->listen(5)){
		cout << "server listen error!" << endl;
		return 1;
	}else
		cout << "server listened successed!" << endl;

	while(client = server->accept()){
		cout << "client " << client->getAddr() << ':' << client->getPort() << " has connected!" << endl;
		
		proxy.setSocket(client);

		TransmissionData data;
		data.add("ip",client->getAddr());
		data.add("port",client->getPort());

		if(proxy.write(data))
			cout << "server writed successed!" << endl;
		else
			cout << "server writed failed!" << endl;
	}
	return 0;
}

