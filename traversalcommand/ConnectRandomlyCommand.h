#ifndef CONNECT_RANDOMLY_COMMAND_H
#define CONNECT_RANDOMLY_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ConnectRandomlyCommand : public TraversalCommand
{
public:
    ConnectRandomlyCommand(){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type){ }
};

LIB_END

#endif // CONNECT_RANDOMLY_COMMAND_H
