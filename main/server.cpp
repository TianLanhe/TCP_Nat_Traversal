#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/socket/ServerSocket.h"

#include <iostream>
#include <string>

using namespace std;
using namespace Lib;

ServerSocket::port_type stringToShort(const string& str){
    ServerSocket::port_type port = 0;
    for(string::size_type i = 0;i<str.size();++i){
        port = port * 10 + str[i] - '0';
    }
    return port;
}

int main(int argc,char *argv[]){
	if(argc != 2){
		cout << "Usage: server port" << endl;
		return 1;
	}
	
	ServerSocket::port_type port = stringToShort(argv[1]);
	
	ServerSocket *server = ReuseSocketFactory::GetInstance()->GetReuseServerSocket();

	ClientSocket *client;
	TransmissionProxy proxy;

	if(!server->bind(port)){
		cout << "server bind " << port << " error!" << endl;
		return 1;
	}else
		cout << "server has bound " << port << " !"<< endl;

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

