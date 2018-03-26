#ifndef NAT_CHECKER_SERVER_H
#define NAT_CHECKER_SERVER_H

#include "../Object.h"
#include "NatCheckerCommon.h"
#include "../socket/ServerSocket.h"

#include <string>

LIB_BEGIN

#define DEFAULT_LISTEN_NUM 10

class ClientSocket;

class NatCheckerServer : public Object
{
public:
    typedef ServerSocket::port_type port_type;

public:
    NatCheckerServer(const std::string&, port_type, const std::string&, port_type);
    ~NatCheckerServer();

    bool setListenNum(size_t);

    // this function will run forever, you should call this function in a new thread
    void waitForClient();

protected:
    static void handle_request(NatCheckerServer*,ClientSocket*);

    size_t _getMaxTryTime(){ return 3; }

private:
    ServerSocket *m_main_server;

    std::string m_another_addr;
    std::string m_main_addr;
    port_type m_main_port;
    port_type m_another_port;

};

LIB_END

#endif // NAT_CHECKER_SERVER_H
