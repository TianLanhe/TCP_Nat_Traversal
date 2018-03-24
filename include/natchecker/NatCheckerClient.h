#ifndef NAT_CHECKER_CLIENT_H
#define NAT_CHECKER_CLIENT_H

#include "../Object.h"
#include "NatCheckerCommon.h"
#include "../socket/ClientSocket.h"

#include <string>
#include <vector>

LIB_BEGIN

class NatCheckerClient : public Object
{
public:
    typedef typename ClientSocket::port_type port_type;

public:
    NatCheckerClient(const std::string&, port_type);
    ~NatCheckerClient();

    // Review: maybe should spilt into several operation
    bool connect(const std::string& addr, port_type port);

    bool bind(const std::string& addr, port_type port);
    bool isBound() const;

    nat_type getNatType() const { return m_natType; }

    std::string getExternAddr() const;
    port_type getExternPort() const;

    std::string getLocalAddr() const;
    port_type getLocalPort() const;

protected:
    NatCheckerClient(const NatCheckerClient&);	// 禁止拷贝和赋值
    NatCheckerClient& operator=(const NatCheckerClient&);

    ClientSocket* _getMainSocket() const { return m_sockets.empty() ? NULL : m_sockets[0]; }

    bool isConnected() const { return m_isConnected; }

private:
    std::vector<ClientSocket*> m_sockets;
    nat_type m_natType;

    std::string m_ext_addr;
    port_type m_ext_port;

    bool m_isConnected;
};

LIB_END

#endif // NAT_CHECKER_CLIENT_H
