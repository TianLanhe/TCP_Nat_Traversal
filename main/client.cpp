#include "../include/nattraversal/NatTraversalClient.h"
#include "../include/socket/ClientSocket.h"

#include <iostream>
#include <string>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#endif

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
	while (1) {
		socket = ext_client->waitForPeerHost();

		if (socket)
			echoSocketAddr(socket);
		else
			cout << "waitForPeerHost return NULL" << endl;
	}
}

int main(int argc,char *argv[]){
    if(argc < 3){
        cout << "Usage: " << argv[0] << " <Identify> <ServerIP> [ServerPort]" << endl;
        return 1;
    }

#if defined(_WIN32) || defined(_WIN64)
    WORD socket_version;
    WSADATA wsadata;
    socket_version = MAKEWORD(2, 2);
    if (WSAStartup(socket_version, &wsadata) != 0){
        cout << "WSAStartup error!" << endl;
        return 1;
    }
#endif

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
