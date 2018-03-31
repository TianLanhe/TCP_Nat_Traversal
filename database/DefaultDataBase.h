#ifndef DEFAULT_DATA_BASE_H
#define DEFAULT_DATA_BASE_H

#include "../include/database/DataBase.h"
#include "../include/observer/Subject.h"

#include <map>
#include <mutex>
#include <cstring>

LIB_BEGIN

// 要求存储的记录对象有 getIdentifier 这个函数
template < typename T >
class DefaultDataBase : public DataBase<T> , public Subject
{
public:
    typedef typename DataBase<T>::Members Members;

public:
    virtual bool hasRecord(const std::string& identifier) const;
    virtual Members getMembers() const;

    virtual T& getRecord(const std::string& identifier);
    virtual const T& getRecord(const std::string& identifier) const;
	
    virtual bool addRecord(const T&);	// 因为这里只是简单的用 map 存储，所以添加时若存在之前的记录，则会覆盖之前存在的记录
	virtual bool removeRecord(const std::string&);

private:
    void notifyChange(const std::string&);
	
private:
    std::map<std::string, T> m_map;
    mutable std::mutex m_mutex;
};

template < typename T >
bool DefaultDataBase<T>::hasRecord(const std::string& identifier) const {
    m_mutex.lock();

    bool ret = m_map.find(identifier) != m_map.end();

    m_mutex.unlock();
    return ret;
}

template < typename T >
T& DefaultDataBase<T>::getRecord(const std::string& identifier){
    m_mutex.lock();

    T& ret = m_map.at(identifier);

    m_mutex.unlock();
    return ret;
}

template < typename T >
const T& DefaultDataBase<T>::getRecord(const std::string& identifier) const {
    m_mutex.lock();

    const T& ret = m_map.at(identifier);

    m_mutex.unlock();
    return ret;
}

template < typename T >
typename DefaultDataBase<T>::Members DefaultDataBase<T>::getMembers() const {
    Members members;
    m_mutex.lock();

    for(auto it = m_map.begin();it != m_map.end();++it)
        members.push_back(it->first);

    m_mutex.unlock();
    return members;
}

template < typename T >
void DefaultDataBase<T>::notifyChange(const std::string& identifier){
    char buf[512] = {0};
    strcpy(buf,identifier.c_str());

    notifyAll((void*)buf);
}

template < typename T >
bool DefaultDataBase<T>::addRecord(const T& record) {
    m_mutex.lock();

    if(m_map.find(record.getIdentifier()) != m_map.end())
    //	return false;
        m_map.erase(record.getIdentifier());

    m_map.insert(std::pair<std::string,T>(record.getIdentifier(),record));

    m_mutex.unlock();

    notifyChange(record.getIdentifier());
    return true;
}

template < typename T >
bool DefaultDataBase<T>::removeRecord(const std::string& identifier){
    m_mutex.lock();

    if(m_map.find(identifier) == m_map.end())
        return false;

    m_map.erase(identifier);

    m_mutex.unlock();

    notifyChange(identifier);

    return true;
}

LIB_END

#endif // !DEFAULT_DATA_BASE_H

