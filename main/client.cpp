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
    if(argc < 3){
        cout << "Usage: " << argv[0] << " <Identify> <ServerIP> [ServerPort]" << endl;
        return 1;
    }

    NatTraversalClient client(argv[1]);
	
    short port = atoi(argv[2]);
    if(!client.enroll(argv[1],port)){
        cerr << "client enroll to traversal server failed" << endl;
		return 1;
    }
		
    ext_client = &client;
	
    thread(func).detach();
	
    while(1){
        ClientSocket *socket = client.connectToPeerHost("client2");

        if(socket == NULL){
            cout << "connectToPeerHost return NULL" << endl;
            return 0;
        }

        echoSocketAddr(socket);
        delete socket;
    }

    return 0;
}
