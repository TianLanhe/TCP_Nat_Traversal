#ifndef LISTEN_AND_PUNCH_COMMAND_H
#define LISTEN_AND_PUNCH_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ListenAndPunchCommand : public TraversalCommand
{
public:
    ListenAndPunchCommand(){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type){ }
};

LIB_END

#endif // !LISTEN_AND_PUNCH_COMMAND_H
