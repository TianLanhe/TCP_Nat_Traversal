#ifndef LISTEN_DIRECTLY_COMMAND_H
#define LISTEN_DIRECTLY_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ListenDirectlyCommand : public TraversalCommand
{
public:
    ListenDirectlyCommand(){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type);
};

LIB_END

#endif // !LISTEN_DIRECTLY_COMMAND_H
