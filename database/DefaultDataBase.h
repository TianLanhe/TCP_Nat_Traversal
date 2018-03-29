#ifndef DEFAULT_DATA_BASE_H
#define DEFAULT_DATA_BASE_H

#include "../include/database/DataBase.h"

#include <map>

LIB_BEGIN

class DefaultDataBase : public DataBase
{
public:
	virtual bool hasRecord(const std::string&) const;
	virtual DataRecord getRecord(const std::string&) const;
	
	virtual bool addRecord(const DataRecord&);
	virtual bool removeRecord(const std::string&);
	
private:
	std::map<std::string, DataRecord> m_map;
};

LIB_END

#endif // !DEFAULT_DATA_BASE_H

