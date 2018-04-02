#ifndef LISTEN_AND_PUNCH_RANDOMLY_COMMAND_H
#define LISTEN_AND_PUNCH_RANDOMLY_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ListenAndPunchRandomlyCommand : public TraversalCommand
{
public:
    ListenAndPunchRandomlyCommand(){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type){ }
};

LIB_END

#endif // !LISTEN_AND_PUNCH_RANDOMLY_COMMAND_H
