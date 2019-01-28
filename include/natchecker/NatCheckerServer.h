#ifndef NAT_CHECKER_SERVER_H
#define NAT_CHECKER_SERVER_H

#include "../Object.h"
#include "NatCheckerCommon.h"

#include <string>

LIB_BEGIN

#define STUN_MAIN_PORT 6666
#define STUN_SECONDARY_PORT 8888

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

template < typename T >
class DataBase;

class NatCheckerServer : public Object
{
public:
    NatCheckerServer():m_database(NULL){}

    // use database object to store and update the information of nat type which traversal server needs
    virtual bool setDataBase(DataBase<DataRecord>* database){
        CHECK_PARAMETER_EXCEPTION(database);

        if(m_database)
            return false;

        m_database = database;
        return true;
    }

    // this function will run forever, you should call this function in a new thread
    virtual void waitForClient() = 0;

protected:
    DataBase<DataRecord> *m_database;
};

LIB_END

#endif // NAT_CHECKER_SERVER_H
