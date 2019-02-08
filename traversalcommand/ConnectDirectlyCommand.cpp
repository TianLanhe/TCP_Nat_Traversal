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
// ���ﲻ��ֱ�ӷ����� connect ���ɴκ�ֱ�ӷ��أ��� select һ�£���������ʵ�������ˣ����Ƿ����� connect ���� -1 ��Ϊ����������
    if(!ret->connect(data.getString(DESTINY_IP),data.getInt(DESTINY_PORT),MAX_CONNECT_TRY_TIME)){
        delete ret;
        return NULL;
    }
    ret->setNonBlock(false);

    return ret;

// Review: ������� select �ɹ��ˣ����� ClientSocket ��ʵ���� isConnect �� false���в�һ�µ����⣬������Ȳ���ô�����
//    // ֱ�����Ͼ�ֱ�ӷ�����
//    if(ret->connect(data.getString(DESTINY_IP),data.getInt(DESTINY_PORT),MAX_CONNECT_TRY_TIME)){
//        ret->setNonBlock(false);
//        return ret;
//    }

//    // û���� select һ��
//    SmartPointer<SocketSelector> selector(SocketSelectorFactory::GetInstance()->GetWriteSelector());
//    selector->add(ret);
//    SocketSelector::SocketVector sockets = selector->select(CONNECT_WAIT_TIME);

//    // select ����û�취��
//    if(sockets.empty()){
//        delete ret;
//        return NULL;
//    }else{
//        ret->setNonBlock(false);
//        return ret;
//    }
}
