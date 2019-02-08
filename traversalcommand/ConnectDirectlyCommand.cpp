#include "ConnectDirectlyCommand.h"
#include "include/socket/ReuseSocketFactory.h"
#include "include/socket/ClientSocket.h"
#include "include/socket/SocketSelector.h"
#include "include/socket/SocketSelectorFactory.h"
#include "include/SmartPointer.h"

using namespace Lib;

#define MAX_CONNECT_TRY_TIME 100
#define CONNECT_WAIT_TIME 1.0

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
// 这里不能直接非阻塞 connect 若干次后直接返回，得 select 一下，否则容易实际连上了，但是非阻塞 connect 返回 -1 因为正在连接中
    if(!ret->connect(data.getString(DESTINY_IP),data.getInt(DESTINY_PORT),MAX_CONNECT_TRY_TIME)){
        delete ret;
        return NULL;
    }
    ret->setNonBlock(false);

    return ret;

// Review: 如果这里 select 成功了，但是 ClientSocket 类实际上 isConnect 是 false，有不一致的问题，这里就先不这么处理吧
//    // 直接连上就直接返回了
//    if(ret->connect(data.getString(DESTINY_IP),data.getInt(DESTINY_PORT),MAX_CONNECT_TRY_TIME)){
//        ret->setNonBlock(false);
//        return ret;
//    }

//    // 没连上 select 一下
//    SmartPointer<SocketSelector> selector(SocketSelectorFactory::GetInstance()->GetWriteSelector());
//    selector->add(ret);
//    SocketSelector::SocketVector sockets = selector->select(CONNECT_WAIT_TIME);

//    // select 不到没办法了
//    if(sockets.empty()){
//        delete ret;
//        return NULL;
//    }else{
//        ret->setNonBlock(false);
//        return ret;
//    }
}
