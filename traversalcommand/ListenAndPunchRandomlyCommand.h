#ifndef LISTEN_AND_PUNCH_RANDOMLY_COMMAND_H
#define LISTEN_AND_PUNCH_RANDOMLY_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

#include <vector>

LIB_BEGIN

class ListenAndPunchRandomlyCommand : public TraversalCommand
{
public:
    ListenAndPunchRandomlyCommand():shouldPunch(true){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type);

private:
    static void punching(ListenAndPunchRandomlyCommand*, std::vector<ClientSocket*>, int, const ip_type &, port_type);

    int _nrand(int n);
    int _getRandomNum(int start,int length);

    bool shouldPunch;
};

LIB_END

#endif // !LISTEN_AND_PUNCH_RANDOMLY_COMMAND_H
