#ifndef MULT_NAT_CHECKER_SERVER_SLAVE_H
#define MULT_NAT_CHECKER_SERVER_SLAVE_H

#include "include/natchecker/NatCheckerServer.h"
#include "natchecker/MultNatCheckerServerCommon.h"

LIB_BEGIN

class ServerSocket;
class ClientSocket;

class MultNatCheckerServerSlave : public NatCheckerServer
{
public:
    MultNatCheckerServerSlave(port_type port = SLAVE_LISTEN_PORT);
    ~MultNatCheckerServerSlave();

    bool setListenNum(size_t);

    // this function will run forever, you should call this function in a new thread
    void waitForClient();

protected:
    void handle_request(ClientSocket*);

    size_t _getConnectRetryTime(){ return 1; }

protected:
    ServerSocket *m_server;
    port_type m_port;
};

LIB_END

#endif // MULT_NAT_CHECKER_SERVER_SLAVE_H
