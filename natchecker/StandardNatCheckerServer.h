#ifndef STANDARD_NAT_CHECKER_SERVER_H
#define STANDARD_NAT_CHECKER_SERVER_H

#include "../include/natchecker/NatCheckerServer.h"
#include "../include/socket/ServerSocket.h"

#include <vector>

LIB_BEGIN

class ClientSocket;
class DefaultClientSocket;

class StandardNatCheckerServer : public NatCheckerServer
{
public:
    StandardNatCheckerServer(const ip_type&, port_type, const ip_type&, port_type);
    ~StandardNatCheckerServer();

    bool setListenNum(size_t);

    // this function will run forever, you should call this function in a new thread
    void waitForClient();

protected:
    void handle_request(ClientSocket*);

    size_t _getMaxTryTime(){ return 3; }
    size_t _getConnectRetryTime(){ return 1; }

private:
    ServerSocket *m_main_server;

    std::string m_another_addr;
    std::string m_main_addr;
    port_type m_main_port;
    port_type m_another_port;

    std::vector<DefaultClientSocket*> m_clientVec;
};

LIB_END

#endif // STANDARD_NAT_CHECKER_SERVER_H
