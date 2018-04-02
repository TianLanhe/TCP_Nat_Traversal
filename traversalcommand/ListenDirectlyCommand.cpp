#include "ListenDirectlyCommand.h"
#include "../include/socket/ClientSocket.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/socket/ServerSocket.h"
#include "../include/SmartPointer.h"
#include "../socket/ReuseServerSocket.h"

#include <sys/select.h>

using namespace Lib;

ClientSocket* ListenDirectlyCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port)
{
    if(data.getInt(TYPE) != LISTEN_DIRECTLY)
        return NULL;

    SmartPointer<ServerSocket> server(ReuseSocketFactory::GetInstance()->GetServerSocket());

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
    int ret = select(reuseSocket->_getfd()+1,&set,NULL,NULL,&t);

    if(ret == -1){
        THROW_EXCEPTION(ErrorStateException,"select error in NatTraversalServer::waitForClient");
    }else if(ret == 0){
        return NULL;
    }else{
        CHECK_STATE_EXCEPTION(ret == 1);

        ClientSocket *client = server->accept();

        return client;
    }
}
