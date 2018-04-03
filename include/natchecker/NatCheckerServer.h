#ifndef NAT_CHECKER_SERVER_H
#define NAT_CHECKER_SERVER_H

#include "../Object.h"
#include "NatCheckerCommon.h"
#include "../socket/ServerSocket.h"

#include <string>

LIB_BEGIN

struct Address{
    Address():port(0){ }
    Address(const Object::ip_type& ip, Object::port_type port):ip(ip),port(port){ }

    Object::ip_type ip;
    Object::port_type port;
};

class DataRecord : public Object
{
public:
    std::string getIdentifier() const { return m_identifier; }
    Address getExtAddress() const { return m_extAddr; }
    Address getLocalAddress() const { return m_localAddr; }
    nat_type getNatType() const { return m_natType; }

    void setIdentifier(const std::string& identifier) { m_identifier = identifier; }
    void setExtAddress(const Address& addr) { m_extAddr = addr; }
    void setLocalAddress(const Address& addr) { m_localAddr = addr; }
    void setNatType(const nat_type& natType) { m_natType = natType; }

private:
    std::string m_identifier;
    Address m_extAddr;
    Address m_localAddr;
    nat_type m_natType;
};

class ClientSocket;

template < typename T >
class DataBase;

class NatCheckerServer : public Object
{
public:
    NatCheckerServer(const ip_type&, port_type, const ip_type&, port_type);
    ~NatCheckerServer();

    bool setDataBase(DataBase<DataRecord>*);
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

    DataBase<DataRecord> *m_database;
};

LIB_END

#endif // NAT_CHECKER_SERVER_H
