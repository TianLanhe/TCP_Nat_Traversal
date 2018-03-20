#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/socket/ClientSocket.h"

#include <iostream>
#include <string>

using namespace std;
using namespace Lib;

ClientSocket::port_type stringToShort(const string& str){
    ClientSocket::port_type port = 0;
    for(string::size_type i = 0;i<str.size();++i){
        port = port * 10 + str[i] - '0';
    }
    return port;
}

TransmissionData getIpAndPort(const string& clientIp,ClientSocket::port_type clientPort,const string& serverIp,ClientSocket::port_type serverPort){
    TransmissionData ret;

    ClientSocket *client = ReuseSocketFactory::GetInstance()->GetReuseClientSocket();
    if(!client->bind(clientPort,clientIp)){
        cout << "client bind " << clientIp << ":" << clientPort << " error!" << endl;
        return ret;
    }

    if(!client->connect(serverIp,serverPort)){
        cout << "client connect to " << serverIp << ":" << serverPort << " error!" << endl;
        return ret;
    }

    TransmissionProxy proxy(client);

    ret = proxy.read();

    return ret;
}

void output(const string& ip,ClientSocket::port_type port){
    cout << "#####################" << endl;
    cout << "Client's IP and Port:" << endl;
    cout << "IP: " << ip << endl;
    cout << "Port: " << port << endl;
    cout << "#####################" << endl;
}

int main(int argc,char *argv[]){
	if(argc != 3){
        cout << "Usage: client clientIp clientPort" << endl;
		return 1;
	}

    string clientIp = argv[1];
    string strPort = argv[2];
    ClientSocket::port_type clientPort = stringToShort(strPort);

    string serverIp;
    ClientSocket::port_type serverPort = 0;

    cout << "Please input the first ip:port of server." << endl;
    cout << "IP: ";
    cin >> serverIp;
    cout << "Port: ";
    cin >> serverPort;

    output(clientIp,clientPort);

    TransmissionData data = getIpAndPort(clientIp,clientPort,serverIp,serverPort);
    output(data.getString("ip"),data.getInt("port"));

    if(clientIp == data.getString("ip") && clientPort == data.getInt("port")){
        cout << "do not have NAT." << endl;
        return 0;
    }

    cout << "Please input the second ip:port of server." << endl;
    cout << "IP: ";
    cin >> serverIp;
    cout << "Port: ";
    cin >> serverPort;

    TransmissionData data2 = getIpAndPort(clientIp,clientPort,serverIp,serverPort);
    output(data2.getString("ip"),data2.getInt("port"));

    if(data2.getString("ip") == data.getString("ip") && data2.getInt("port") == data.getInt("port")){
        cout << "Endpoint independent." << endl;
        return 0;
    }

    cout << "Please input the third ip:port of server." << endl;
    cout << "IP: ";
    cin >> serverIp;
    cout << "Port: ";
    cin >> serverPort;

    TransmissionData data3 = getIpAndPort(clientIp,clientPort,serverIp,serverPort);
    output(data3.getString("ip"),data3.getInt("port"));

    if(data2.getString("ip") == data3.getString("ip") && data2.getInt("port") == data3.getInt("port")){
        cout << "Address dependent." << endl;
    }else{
        cout << "Address and Port dependent." << endl;
    }

	return 0;
}


