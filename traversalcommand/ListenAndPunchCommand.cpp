#include "ListenAndPunchCommand.h"
#include "../include/socket/ServerSocket.h"
#include "../include/socket/ClientSocket.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/SmartPointer.h"
#include "../socket/ReuseServerSocket.h"

#include <sys/select.h>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace Lib;

void ListenAndPunchCommand::punching(ListenAndPunchCommand *command, ClientSocket *socket, const ip_type& destiny_ip, port_type destiny_port)
{
    while(command->shouldPunch){
        socket->connect(destiny_ip,destiny_port,1);
        sleep(1);
    }
}

ClientSocket* ListenAndPunchCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(DESTINY_PORT))
        return NULL;

    if(data.getInt(TYPE) != LISTEN_BY_PUNCHING_A_HOLE)
        return NULL;

    SmartPointer<ServerSocket> server(ReuseSocketFactory::GetInstance()->GetServerSocket());
    SmartPointer<ClientSocket> client(ReuseSocketFactory::GetInstance()->GetClientSocket());

    if(!client->setNonBlock() || !client->bind(ip,port))
        return NULL;

    if(!server->bind(ip,port))
        return NULL;

    if(!server->listen(1))
        return NULL;

    ReuseServerSocket *reuseSocket = dynamic_cast<ReuseServerSocket*>(server.get());
    if(reuseSocket == NULL)
        return NULL;

    struct timeval t = {5,0};
    fd_set set;
    FD_ZERO(&set);
    FD_SET(reuseSocket->_getfd(),&set);

    thread punch_thread(ListenAndPunchCommand::punching,this,client.get(),data.getString(DESTINY_IP),data.getInt(DESTINY_PORT));

    int ret = select(reuseSocket->_getfd()+1,&set,NULL,NULL,&t);

    if(ret == -1){
        THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalServer::waitForClient");
    }else if(ret == 0){
        shouldPunch = false;
        punch_thread.join();
        return NULL;
    }else{
        CHECK_STATE_EXCEPTION(ret == 1);

        ClientSocket *client = server->accept();

        shouldPunch = false;
        punch_thread.join();

        return client;
    }
}
