#include "../include/nattraversal/NatTraversalClient.h"
#include "../include/socket/ClientSocket.h"

#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace Lib;

NatTraversalClient *ext_client;

void echoSocketAddr(ClientSocket *socket){
	cout << "ip: " << socket->getAddr() << endl;
	cout << "port: " << socket->getPort() << endl;
	cout << "peer ip: " << socket->getPeerAddr() << endl;
	cout << "peer port: " << socket->getPeerPort() << endl;
}

void func(){
	ClientSocket *socket;
	socket = ext_client->waitForPeerHost();
	
	if(socket == NULL){
		cout << "waitForPeerHost return NULL" << endl;
		return;
	}
	
	echoSocketAddr(socket);
}

int main(int argc,char *argv[]){
	NatTraversalClient client("client1");
	
	if(!client.enroll("172.16.1.222",9999))
		return 1;
		
	ext_client = &client;
	
	thread(func).detach();
	
	ClientSocket *socket = client.connectToPeerHost("client2");
	
	if(socket == NULL){
		cout << "connectToPeerHost return NULL" << endl;
		return 0;
	}
	
	echoSocketAddr(socket);

    return 0;
}
