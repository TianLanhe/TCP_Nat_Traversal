#ifndef CONNECT_AROUND_COMMAND_H
#define CONNECT_AROUND_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ConnectAroundCommand : public TraversalCommand
{
public:
    ConnectAroundCommand(){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type);
};

LIB_END

#endif // CONNECT_AROUND_COMMAND_H
