#include "ConnectRandomlyCommand.h"
#include "../include/socket/ClientSocket.h"
#include "../include/socket/ReuseSocketFactory.h"

#include <cstdlib>
#include <unistd.h>

using namespace std;
using namespace Lib;

int ConnectRandomlyCommand::_nrand(int n){
    if(n < 0)
        n = 0;
    else if(n > RAND_MAX)
        n = RAND_MAX;

    int r;
    int bucket_size = RAND_MAX / n;
    do{
        r = rand() / bucket_size;
    }while( r >= n );

    return r;
}

int ConnectRandomlyCommand::_getRandomNum(int start,int length){
    if(start < 0)
        start = 0;
    else if(start > RAND_MAX)
        start = RAND_MAX;

    if(RAND_MAX - start >= length)
        length = RAND_MAX - start;

    return _nrand(length) + start;
}

ClientSocket* ConnectRandomlyCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port){
    if(!data.isMember(DESTINY_IP) || !data.isMember(TRY_TIME))
        return NULL;

    if(data.getInt(TYPE) != CONNECT_RANDOMLY)
        return NULL;

    ip_type destiny_ip = data.getString(DESTINY_IP);

    int try_time = data.getInt(TRY_TIME);

    if(try_time < 2)
        return NULL;

    ClientSocket *ret = ReuseSocketFactory::GetInstance()->GetClientSocket();

    if(!ret->bind(ip,port)){
        delete ret;
        return NULL;
    }

    usleep(500000);

    for(int count=0;count < try_time;++count){
        if(ret->connect(destiny_ip,_getRandomNum(1024,64512),1)){
            return ret;
        }
    }

    delete ret;
    return NULL;
}
