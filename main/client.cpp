#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../include/socket/DefaultSocketFactory.h"
#include "../include/socket/ClientSocket.h"

#include <iostream>
#include <string>

using namespace std;
using namespace Lib;

int main(int argc,char *argv[]){
	if(argc != 3){
		cout << "Usage: client serverIp serverPort" << endl;
		return 1;
	}

	string ip = argv[1];
	string strPort = argv[2];
	
	ClientSocket::port_type port = 0;
	for(string::size_type i = 0;i<strPort.size();++i){
		port = port * 10 + strPort[i] - '0';
	}	

	ClientSocket *client = DefaultSocketFactory::GetInstance()->GetDefaultClientSocket();

	if(!client->connect(ip,port)){
		cout << "connect to " << ip << " " << port << " error!" << endl;
		return 1;	
	}

	TransmissionProxy proxy(client);

	TransmissionData data = proxy.read();

	cout << "ip: " << client->getAddr() << endl;
	cout << "port: " << client->getPort() << endl;
	cout << "peer_ip: " << data["ip"] << endl;
	cout << "peer_port: " << data["port"] << endl;

	return 0;
}


