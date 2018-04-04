#include "../include/traversalcommand/TraversalCommand.h"
#include "ConnectDirectlyCommand.h"
#include "ConnectAroundCommand.h"
#include "ConnectRandomlyCommand.h"
#include "ListenAndPunchCommand.h"
#include "ListenAndPunchRandomlyCommand.h"
#include "ListenDirectlyCommand.h"

using namespace std;
using namespace Lib;

#define UNKNOWN TraversalCommand::UNKNOWN
#define CONNECT_DIRECTLY TraversalCommand::CONNECT_DIRECTLY
#define CONNECT_AROUND TraversalCommand::CONNECT_AROUND
#define CONNECT_RANDOMLY TraversalCommand::CONNECT_RANDOMLY
#define LISTEN_DIRECTLY TraversalCommand::LISTEN_DIRECTLY
#define LISTEN_BY_PUNCHING_A_HOLE TraversalCommand::LISTEN_BY_PUNCHING_A_HOLE
#define LISTEN_BY_PUNCHING_SOME_HOLE TraversalCommand::LISTEN_BY_PUNCHING_SOME_HOLE

#define map_type nat_type::map_type
#define filter_type nat_type::filter_type

#define ENDPOINT_INDEPENDENT nat_type::ENDPOINT_INDEPENDENT
#define ADDRESS_DEPENDENT nat_type::ADDRESS_DEPENDENT
#define ADDRESS_AND_PORT_DEPENDENT nat_type::ADDRESS_AND_PORT_DEPENDENT

bool isNatType(const nat_type& natType,const map_type& mapType,const filter_type& filterType){
    return natType.getMapType() == mapType && natType.getFilterType() == filterType;
}

TraversalCommand::Types Lib::GetTraversalType(const nat_type& natType1,const nat_type& natType2){
    TraversalCommand::Types ret;

/*
	NAT所有类型：
	无 NAT
	无关-无关	地址-无关	端口-无关
	无关-地址	地址-地址	端口-地址
	无关-端口	地址-端口	端口-端口

	其中 Map 类型为地址相关型或端口相关型的可进一步分为可预测与不可预测两类
*/

	// 若一方是无 NAT 的，则无论对方是什么类型，都是无 NAT 的一方监听，另一方直接连接
    if(!natType2.haveNat())
    {
        ret.push_back(CONNECT_DIRECTLY);
        ret.push_back(LISTEN_DIRECTLY);
    }
    else if(!natType1.haveNat())
    {
        ret.push_back(LISTEN_DIRECTLY);
        ret.push_back(CONNECT_DIRECTLY);
    }
    // 若一方是 XX-无关 型，即 Map 类型任意，Filter 类型是终端无关型的，则无论对方是什么类型，都是 XX-无关 型的一方监听，另一方直接连接
    else if(natType2.getFilterType() == ENDPOINT_INDEPENDENT)
    {
        ret.push_back(CONNECT_DIRECTLY);
        ret.push_back(LISTEN_DIRECTLY);
    }
    else if(natType1.getFilterType() == ENDPOINT_INDEPENDENT)
    {
        ret.push_back(LISTEN_DIRECTLY);
        ret.push_back(CONNECT_DIRECTLY);
    }
    // 若一方是 无关-地址 型，即 Map 类型是终端无关型，Filter类型是地址相关型的，则无论对方是什么类型，都是 无关-地址 型的一方向对方打一个洞并监听，另一方直接发起连接
    else if(isNatType(natType2,ENDPOINT_INDEPENDENT,ADDRESS_DEPENDENT))	
    {
        ret.push_back(CONNECT_DIRECTLY);
        ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
    }
    else if(isNatType(natType1,ENDPOINT_INDEPENDENT,ADDRESS_DEPENDENT))
    {
        ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
        ret.push_back(CONNECT_DIRECTLY);
    }
    // 若一方是 无关-端口 型，另一方也是 无关-端口 型，则其中一方打洞监听，另一方直接连接
    else if(isNatType(natType1,ENDPOINT_INDEPENDENT,ADDRESS_AND_PORT_DEPENDENT) && isNatType(natType2,ENDPOINT_INDEPENDENT,ADDRESS_AND_PORT_DEPENDENT))
    {
        ret.push_back(CONNECT_DIRECTLY);
        ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
    }
    // 若一方是 无关-端口 型，另一方是 非无关-非无关 型，则分为两种情况：
    // 一方是 无关-端口 型，另一方为 (可预测)非无关-非无关 型，则后者向前者打洞，前者向后者发起连接，为提高成功率，前者连接时采用扫描连接的方式
    // 一方是 无关-端口 型，另一方为 (不可预测)非无关-非无关 型，则后者向前者打 n 个洞，前者随机修改目的端口尝试 n 个连接
    else if(isNatType(natType1,ENDPOINT_INDEPENDENT,ADDRESS_AND_PORT_DEPENDENT))
    {
        if(natType2.isPredictable())
        {
            ret.push_back(CONNECT_AROUND);
            ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
        }
        else
        {
            ret.push_back(CONNECT_RANDOMLY);
            ret.push_back(LISTEN_BY_PUNCHING_SOME_HOLE);
        }
    }
    else if(isNatType(natType2,ENDPOINT_INDEPENDENT,ADDRESS_AND_PORT_DEPENDENT))
    {
        if(natType1.isPredictable())
        {
            ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
            ret.push_back(CONNECT_AROUND);
        }
        else
        {
            ret.push_back(LISTEN_BY_PUNCHING_SOME_HOLE);
            ret.push_back(CONNECT_RANDOMLY);
        }
    }
    
    ///////////////////////////////////////////// 双方都是对称型 NAT
    
    
    // 若一方是 非无关-地址 型，另一方为任意对称型，则分为两种情况：
    // 一方是 (可预测)非无关-地址 型，另一方为任意对称型，则前者向后者打洞监听，后者向前者发起连接，为提高成功率，后者连接时采用扫描连接的方式
    // 一方时 (不可预测)非无关-地址 型，另一方为任意对称型，则前者向后者打 n 个洞，后者随机修改目的端口尝试 n 个连接
    else if(isNatType(natType2,ADDRESS_DEPENDENT,ADDRESS_DEPENDENT) || isNatType(natType2,ADDRESS_AND_PORT_DEPENDENT,ADDRESS_DEPENDENT))
    {
        if(natType2.isPredictable())
        {
            ret.push_back(CONNECT_AROUND);
            ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
        }
        else
        {
            ret.push_back(CONNECT_RANDOMLY);
            ret.push_back(LISTEN_BY_PUNCHING_SOME_HOLE);
        }
    }
    else if(isNatType(natType1,ADDRESS_DEPENDENT,ADDRESS_DEPENDENT) || isNatType(natType1,ADDRESS_AND_PORT_DEPENDENT,ADDRESS_DEPENDENT))
    {
        if(natType1.isPredictable())
        {
            ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
            ret.push_back(CONNECT_AROUND);
        }
        else
        {
            ret.push_back(LISTEN_BY_PUNCHING_SOME_HOLE);
            ret.push_back(CONNECT_RANDOMLY);
        }
    }
    // 只剩下 “地址-端口” 与 “端口-端口” 还没考虑
    // 若一方是 (可预测)非无关-端口 型，另一方也是 (可预测)非无关-端口 型，则一方监听并向特定目的地址打洞，另一方向对方直接发起连接
    // 这里无法为提高成功率做任何措施，可以用重试的方法提高正确率
    else if((isNatType(natType1,ADDRESS_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT) || isNatType(natType1,ADDRESS_AND_PORT_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT))
            && natType1.isPredictable() &&
            (isNatType(natType2,ADDRESS_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT) || isNatType(natType2,ADDRESS_AND_PORT_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT))
            && natType2.isPredictable())
    {
        ret.push_back(CONNECT_DIRECTLY);
        ret.push_back(LISTEN_BY_PUNCHING_A_HOLE);
    }
    // 若一方是 (可预测)地址-端口 型，另一方是 (不可预测)非无关-端口 型，则后者监听并改变源端口向预测的目的端口打 n 个洞，前者随机改变目的端口尝试 n 次连接
    else if((isNatType(natType2,ADDRESS_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT) || isNatType(natType2,ADDRESS_AND_PORT_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT))
            && !natType2.isPredictable() &&
            isNatType(natType1,ADDRESS_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT)
            && natType1.isPredictable())
    {
        ret.push_back(CONNECT_RANDOMLY);
        ret.push_back(LISTEN_BY_PUNCHING_SOME_HOLE);
    }
    else if((isNatType(natType1,ADDRESS_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT) || isNatType(natType1,ADDRESS_AND_PORT_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT))
            && !natType1.isPredictable() &&
            isNatType(natType2,ADDRESS_DEPENDENT,ADDRESS_AND_PORT_DEPENDENT)
            && natType2.isPredictable())
    {
        ret.push_back(LISTEN_BY_PUNCHING_SOME_HOLE);
        ret.push_back(CONNECT_RANDOMLY);
    }
    // 其他情况，<(可预测)端口-端口，(不可预测)非无关-端口>与<(不可预测)非无关-端口，(不可预测)非无关-端口>
    // 目前没一点法子
    else
    {
        ret.push_back(UNKNOWN);
        ret.push_back(UNKNOWN);
    }

    return ret;
}

TransmissionData Lib::GetTraversalData(const TraversalCommand::TraversalType& type, const nat_type& natType, const Object::ip_type& ip, Object::port_type port){
    TransmissionData ret;
    ret.add(TYPE,(int)(type));
    switch (type) {
    case UNKNOWN:
        break;
    case CONNECT_DIRECTLY:
        ret.add(DESTINY_IP,ip);
        ret.add(DESTINY_PORT,port);
        //ret.add(TRY_TIME,1);
        //ret.add(INCREMENT,0);
        break;
    case CONNECT_AROUND:
        ret.add(DESTINY_IP,ip);
        ret.add(DESTINY_PORT,port);
        ret.add(TRY_TIME,CONNECT_AROUND_TRY_TIME);
        ret.add(INCREMENT,natType.getPortDelta());
        break;
    case CONNECT_RANDOMLY:
        ret.add(DESTINY_IP,ip);
        //ret.add(DESTINY_PORT,port);
        ret.add(TRY_TIME,CONNECT_RANDOMLY_TRY_TIME);
        //ret.add(INCREMENT,0);
        break;
    case LISTEN_DIRECTLY:
        break;
    case LISTEN_BY_PUNCHING_A_HOLE:
        ret.add(DESTINY_IP,ip);
        ret.add(DESTINY_PORT,port);
        break;
    case LISTEN_BY_PUNCHING_SOME_HOLE:
        ret.add(DESTINY_IP,ip);
        ret.add(DESTINY_PORT,port);
        ret.add(TRY_TIME,CONNECT_RANDOMLY_TRY_TIME);
        break;
    }

    return ret;
}

TraversalCommand* Lib::GetTraversalCommandByType(const TraversalCommand::TraversalType& type){
    TraversalCommand *ret = NULL;

    switch(type){
    case UNKNOWN:
        ret = NULL;
        break;
    case CONNECT_DIRECTLY:
        ret = new ConnectDirectlyCommand();
        break;
    case CONNECT_AROUND:
        ret = new ConnectAroundCommand();
        break;
    case CONNECT_RANDOMLY:
        ret =  new ConnectRandomlyCommand();
        break;
    case LISTEN_DIRECTLY:
        ret = new ListenDirectlyCommand();
        break;
    case LISTEN_BY_PUNCHING_A_HOLE:
        ret = new ListenAndPunchCommand();
        break;
    case LISTEN_BY_PUNCHING_SOME_HOLE:
        ret = new ListenAndPunchRandomlyCommand();
        break;
    }

    return ret;
}
