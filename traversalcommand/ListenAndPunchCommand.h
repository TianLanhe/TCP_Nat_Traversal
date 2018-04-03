#ifndef LISTEN_AND_PUNCH_COMMAND_H
#define LISTEN_AND_PUNCH_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ListenAndPunchCommand : public TraversalCommand
{
public:
    ListenAndPunchCommand():shouldPunch(true){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type);

private:
    static void punching(ListenAndPunchCommand*,ClientSocket*,int,const ip_type&,port_type);

    bool shouldPunch;
};

LIB_END

#endif // !LISTEN_AND_PUNCH_COMMAND_H
