#ifndef NAT_CHECKER_CLIENT_H
#define NAT_CHECKER_CLIENT_H

#include "../Object.h"
#include "NatCheckerCommon.h"
#include "../socket/ClientSocket.h"

#include <string>
//#include <vector>

LIB_BEGIN

class NatCheckerClient : public Object
{
public:
    typedef typename ClientSocket::port_type port_type;

public:
    NatCheckerClient(const std::string&,const ip_type&, port_type);
    ~NatCheckerClient();

    bool connect(const ip_type& addr, port_type port);

    bool bind(const ip_type& addr, port_type port);
    bool isBound() const;

    nat_type getNatType() const { return m_natType; }

    ip_type getExternAddr() const;
    port_type getExternPort() const;

    ip_type getLocalAddr() const;
    port_type getLocalPort() const;

protected:
    NatCheckerClient(const NatCheckerClient&);	// 禁止拷贝和赋值
    NatCheckerClient& operator=(const NatCheckerClient&);

    ClientSocket* _getMainSocket() const { return m_socket; /*return m_sockets.empty() ? NULL : m_sockets[0];*/ }

    bool isConnected() const { return m_isConnected; }

private:
    //std::vector<ClientSocket*> m_sockets;	   本来打算存储探测过程中的所有socket，特别是在探测 Map 类型时的发起连接的 socket
    ClientSocket *m_socket;					// 因为怕关闭连接后端口归还给 NAT 后 NAT 会复用这些端口，经测试并没有影响，这里先不存储吧
    nat_type m_natType;

    ip_type m_ext_addr;
    port_type m_ext_port;

    bool m_isConnected;

    std::string m_identifier;
};

LIB_END

#endif // NAT_CHECKER_CLIENT_H
