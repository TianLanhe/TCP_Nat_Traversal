#ifndef CONNECT_DIRECTLY_COMMAND_H
#define CONNECT_DIRECTLY_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ConnectDirectlyCommand : public TraversalCommand
{
public:
    ConnectDirectlyCommand(){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type);
};

LIB_END

#endif // !CONNECT_DIRECTLY_COMMAND_H
