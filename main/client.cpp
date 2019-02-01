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
	
    short port;
    if(argc >= 4){
        port = atoi(argv[3]);
    }else{
        port = 9999;
    }
    if(!client.enroll(argv[2],port)){
        cerr << "client enroll to traversal server failed" << endl;
		return 1;
    }
		
    ext_client = &client;
	
    thread(func).detach();
	
    while(1){
        string identify;
        cout << "Please input the client to connect:";
        cin >> identify;
        ClientSocket *socket = client.connectToPeerHost(identify);

        if(socket == NULL){
            cout << "connectToPeerHost return NULL" << endl;
        }else{
            echoSocketAddr(socket);
            delete socket;
        }
    }

    return 0;
}
