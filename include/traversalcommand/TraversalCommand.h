#ifndef TRAVERSAL_COMMAND_H
#define TRAVERSAL_COMMAND_H

#include "../Object.h"
#include "../NatType.h"
#include "../transmission/TransmissionData.h"

#include <vector>

LIB_BEGIN

#define DESTINY_IP "destinyIp"
#define DESTINY_PORT "destinyPort"

#define TRY_TIME "tryTime"
#define INCREMENT "inc"

#define TYPE "type"

#define TRY_TIME_CONSTANT 450


class ClientSocket;

class TraversalCommand : public Object
{
public:
    enum TraversalType{
        UNKNOWN,				// 无法识别或无法穿越

        CONNECT_DIRECTLY,		// 直接发起连接
        CONNECT_AROUND,			// 扫描式尝试连续 n 个连接
        CONNECT_RANDOMLY,		// 随机式尝试任意 n 个连接

        LISTEN_DIRECTLY,				// 直接监听本地端口
        LISTEN_BY_PUNCHING_A_HOLE,		// 监听本地端口并向特定地址打洞
        LISTEN_BY_PUNCHING_SOME_HOLE,	// 监听本地端口并向特定地址打 n 个洞，一般通过修改本地源端口来打洞
    };

    typedef std::vector<TraversalType> Types;

public:
    virtual ClientSocket* traverse(const TransmissionData&, const ip_type&, port_type) = 0;
};

TraversalCommand::Types GetTraversalType(const nat_type&, const nat_type&);

TransmissionData GetTraversalData(const TraversalCommand::TraversalType&, const nat_type&, const Object::ip_type&, Object::port_type);

TraversalCommand* GetTraversalCommandByType(const TraversalCommand::TraversalType&);

LIB_END

#endif // !TRAVERSAL_COMMAND_H
