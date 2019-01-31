#ifndef MULT_NAT_CHECKER_SERVER_MASTER_H
#define MULT_NAT_CHECKER_SERVER_MASTER_H

#include "include/natchecker/NatCheckerServer.h"

#include <vector>
#include <string>

LIB_BEGIN

class ServerSocket;
class ClientSocket;
class DefaultClientSocket;

class MultNatCheckerServerMaster : public NatCheckerServer
{
public:
    MultNatCheckerServerMaster(port_type main_port = STUN_MAIN_PORT,port_type another_port = STUN_SECONDARY_PORT);
    ~MultNatCheckerServerMaster();

    bool setListenNum(size_t);

    // this function will run forever, you should call this function in a new thread
    void waitForClient();

protected:
    void handle_request(ClientSocket*);

    size_t _getMaxTryTime(){ return 3; }
    size_t _getConnectRetryTime(){ return 1; }

protected:
    std::string m_slaveIp;      // slave 的 IP

    ServerSocket *m_server;
    port_type m_main_port;
    port_type m_another_port;

    std::vector<DefaultClientSocket*> m_clientVec;
};

LIB_END

#endif // MULT_NAT_CHECKER_SERVER_MASTER_H