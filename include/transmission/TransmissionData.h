#ifndef TRANSMISSION_DATA_H
#define TRANSMISSION_DATA_H

#include "../Object.h"

#include <map>
#include <string>
#include <vector>

LIB_BEGIN

class TransmissionData : public Object
{
public:
    typedef unsigned int uint;
    typedef size_t size_type;
    typedef std::vector<std::string> Members;
    typedef std::map<std::string,std::string> Objects;

public:
    bool add(const std::string& key,int value);
    bool add(const std::string& key,uint value);
    bool add(const std::string& key,bool value);
    bool add(const std::string& key,double value);
    bool add(const std::string& key,const std::string& value);
    bool add(const std::string& key,const char *value);

    bool set(const std::string& key,int value);
    bool set(const std::string& key,uint value);
    bool set(const std::string& key,bool value);
    bool set(const std::string& key,double value);
    bool set(const std::string& key,const std::string& value);
    bool set(const std::string& key,const char *value);

    int getInt(const std::string& key) const;
    uint getUInt(const std::string& key) const;
    double getDouble(const std::string& key) const;
    bool getBool(const std::string& key) const;
    std::string getString(const std::string& key) const;

    std::string& operator[](const std::string&);
    const std::string& operator[](const std::string&) const;

    bool remove(const std::string& key);

    bool isMember(const std::string& key) const;
    Members getMembers() const;

    void clear(){ return m_map.clear(); }

    bool empty() const { return m_map.empty(); }
    size_type size() const { return m_map.size(); }

private:
    std::string valueToString(double value) const;
    std::string valueToString(int value) const;
    std::string valueToString(uint value) const;
    std::string valueToString(bool value) const;

private:
    Objects m_map;
};

LIB_END

#endif // TRANSMISSIONDATA_H
