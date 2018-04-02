#include "ConnectAroundCommand.h"
#include "../include/socket/ClientSocket.h"
#include "../include/socket/ReuseSocketFactory.h"

#include <unistd.h>

using namespace Lib;

ClientSocket* ConnectAroundCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(DESTINY_PORT) ||
       !data.isMember(TRY_TIME) || !data.isMember(INCREMENT))
        return NULL;

    if(data.getInt(TYPE) != CONNECT_AROUND)
        return NULL;

    ip_type destiny_ip = data.getString(DESTINY_IP);
    port_type destiny_port = data.getInt(DESTINY_PORT);

    int try_time = data.getInt(TRY_TIME);
    port_type delta = data.getInt(INCREMENT);

    if(try_time < 2 || delta == 0)
        return NULL;

    ClientSocket *ret = ReuseSocketFactory::GetInstance()->GetClientSocket();

    if(!ret->bind(ip,port)){
        delete ret;
        return NULL;
    }

    usleep(500000);

    for(int count=0;count < try_time && count * delta <= MAX_PORT - destiny_port;++count){
        if(ret->connect(destiny_ip,destiny_port + count * delta,1)){
            return ret;
        }
    }

    delete ret;
    return NULL;
}
