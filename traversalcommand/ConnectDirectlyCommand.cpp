#include "ConnectDirectlyCommand.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/socket/ClientSocket.h"

using namespace Lib;

#define MAX_CONNECT_TRY_TIME 100

ClientSocket* ConnectDirectlyCommand::traverse(const TransmissionData& data, const ip_type & ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(DESTINY_PORT))
        return NULL;

    if(data.getInt(TYPE) != CONNECT_DIRECTLY)
        return NULL;

    ClientSocket *ret = ReuseSocketFactory::GetInstance()->GetClientSocket();

    if(!ret->bind(ip,port)){
        delete ret;
        return NULL;
    }

    sleep(CONNECT_SLEEP_TIME);

    ret->setNonBlock();
    if(!ret->connect(data.getString(DESTINY_IP),data.getInt(DESTINY_PORT),MAX_CONNECT_TRY_TIME)){
        delete ret;
        return NULL;
    }
    ret->setNonBlock(false);

    return ret;
}
