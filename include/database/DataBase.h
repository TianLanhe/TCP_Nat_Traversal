#ifndef DATA_BASE_H
#define DATA_BASE_H

#include "../Object.h"

#include <string>
#include <vector>

LIB_BEGIN

template < typename T >
class DataBase : public Object
{
public:
    typedef std::vector<std::string> Members;

public:
    virtual ~DataBase(){ }

	virtual bool hasRecord(const std::string&) const = 0;
    virtual Members getMembers() const = 0;

    virtual T& getRecord(const std::string&) = 0;
    virtual const T& getRecord(const std::string&) const = 0;

    // 只能用于获取存在的键值
    virtual T& operator[](const std::string& key) { return getRecord(key); }
    virtual const T& operator[](const std::string& key) const { return getRecord(key); }
	
    virtual bool addRecord(const T&) = 0;
	virtual bool removeRecord(const std::string&) = 0;
};

LIB_END

#endif // !DATA_BASE_H

