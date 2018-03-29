#ifndef DATA_BASE_H
#define DATA_BASE_H

#include "../Object.h"
#include "../NatType.h"

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

class DataBase : public Object
{
public:
	virtual bool hasRecord(const std::string&) const = 0;
	virtual DataRecord getRecord(const std::string&) const = 0;
	
	virtual bool addRecord(const DataRecord&) = 0;
	virtual bool removeRecord(const std::string&) = 0;
};

LIB_END

#endif // !DATA_BASE_H

