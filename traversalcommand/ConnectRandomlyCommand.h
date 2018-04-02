#ifndef CONNECT_RANDOMLY_COMMAND_H
#define CONNECT_RANDOMLY_COMMAND_H

#include "../include/traversalcommand/TraversalCommand.h"

LIB_BEGIN

class ConnectRandomlyCommand : public TraversalCommand
{
public:
    ConnectRandomlyCommand(){ }

    virtual ClientSocket* traverse(const TransmissionData &, const ip_type &, port_type);

private:
    int _nrand(int n);
    int _getRandomNum(int start,int length);
};

LIB_END

#endif // CONNECT_RANDOMLY_COMMAND_H
