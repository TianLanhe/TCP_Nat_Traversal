#include "include/natchecker/NatCheckerClient.h"
#include "include/socket/ClientSocket.h"
#include "include/socket/ReuseSocketFactory.h"
#include "include/Utility.h"
#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#endif

using namespace std;
using namespace Lib;

int main(int argc,char *argv[]){
    if(argc < 3){
        cout << "Usage: " << argv[0] << " LocalIp CheckerServerIp [CheckerServerPort]" << endl;
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


    string localIp = argv[1];

    string serverIp = argv[2];
    short serverPort = 0;
    if(argc > 3){
        serverPort = atoi(argv[3]);
    }

    if(serverPort == 0)
        serverPort = 6666;

    NatCheckerClient client("test_nat_checker",localIp,Util::getRandomNumByRange(1025,65535));

    cout << "Local Addr: " << client.getLocalAddr() << ":" << client.getLocalPort() << endl;

    if(client.connect(serverIp,serverPort)){
        cout << "Extern Addr: " << client.getExternAddr() << ":" << client.getExternPort() << endl;
        cout << "Mapping Type: " << client.getNatType().getStringMapType() << endl;
        cout << "Filtering Type: " << client.getNatType().getStringFilterType() << endl;
    }else{
        cout << "connect to " << serverIp << ":" << serverPort << " failed" << endl;
        return 1;
    }

    /*ClientSocket *client = ReuseSocketFactory::GetInstance()->GetClientSocket();

    client->bind("10.10.131.131",12341);

    if(client->connect(serverIp,serverPort)){
        cout << "connect to " << serverIp << ":" << serverPort << " successfully" << endl;
    }else{
        cout << "connect to " << serverIp << ":" << serverPort << " failed" << endl;
    }

    delete client;*/

	return 0;
}

