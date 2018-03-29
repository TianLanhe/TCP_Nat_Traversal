#include "DefaultDataBase.h"

using namespace std;
using namespace Lib;

bool DefaultDataBase::hasRecord(const std::string& identifier) const {
	return m_map.find(identifier) != m_map.end();
}

DataRecord DefaultDataBase::getRecord(const std::string& identifier) const {
	return m_map.at(identifier);
}

bool DefaultDataBase::addRecord(const DataRecord& record) {
	if(hasRecord(record.getIdentifier()))
		return false;
		
	m_map.insert(pair<string,DataRecord>(record.getIdentifier(),record));
    return true;
}

bool DefaultDataBase::removeRecord(const std::string& identifier){
	if(!hasRecord(identifier))
		return false;
		
	m_map.erase(identifier);
    return true;
}

