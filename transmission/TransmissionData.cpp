#include "../include/transmission/TransmissionData.h"
#include <cstdio>
#include <cstring>

using namespace Lib;
using namespace std;

string TransmissionData::valueToString(bool value) const {
    return value ? "true" : "false";
}

string TransmissionData::valueToString(double value) const {
    char buffer[65] = {0};
    sprintf(buffer,"%#.16g",value);

    char *pos = buffer + strlen(buffer) - 1;
    if(*pos == '0'){
        while(*pos=='0' && pos >= buffer)
            --pos;

        for(char *point = pos;point >= buffer;--point){
            if(*point == '.'){
                *(pos + 2) = '\0';
                break;
            }
        }
    }

    return buffer;
}

string TransmissionData::valueToString(int value) const {
    char buffer[33] = { 0 };
    char *pos = buffer + 32;

    bool isNegative = ( value < 0 );
    if(isNegative)
        value = -value;

    do{
        *--pos = value % 10 + '0';
        value /= 10;
    }while(value != 0);

    if(isNegative)
        *--pos = '-';

    CHECK_STATE_EXCEPTION(pos - buffer >= 0);

    return pos;
}

string TransmissionData::valueToString(uint value) const {
    char buffer[33] = { 0 };
    char *pos = buffer + 32;

    do{
        *--pos = value % 10 + '0';
        value /= 10;
    }while(value != 0);

    CHECK_STATE_EXCEPTION(pos - buffer >= 0);

    return pos;
}

bool TransmissionData::add(const std::string &key, bool value){
    if(isMember(key))
        return false;

    m_map.insert(pair<string,string>(key,valueToString(value)));

    return true;
}

bool TransmissionData::add(const std::string &key, double value){
    if(isMember(key))
        return false;

    m_map.insert(pair<string,string>(key,valueToString(value)));

    return true;
}

bool TransmissionData::add(const string &key, int value){
    if(isMember(key))
        return false;

    m_map.insert(pair<string,string>(key,valueToString(value)));

    return true;
}

bool TransmissionData::add(const string &key, uint value){
    if(isMember(key))
        return false;

    m_map.insert(pair<string,string>(key,valueToString(value)));

    return true;
}

bool TransmissionData::add(const string &key, const string& value){
    return add(key,value.c_str());
}

bool TransmissionData::add(const string &key, const char* value){
    if(isMember(key))
        return false;

    m_map.insert(pair<string,string>(key,value));

    return true;
}

bool TransmissionData::set(const string &key, bool value){
    if(!isMember(key))
        return false;

    m_map[key] = valueToString(value);

    return true;
}

bool TransmissionData::set(const string &key, const string& value){
    return set(key,value.c_str());
}

bool TransmissionData::set(const string &key, const char *value){
    if(!isMember(key))
        return false;

    m_map[key] = string(value);

    return true;
}

bool TransmissionData::set(const string &key, double value){
    if(!isMember(key))
        return false;

    m_map[key] = valueToString(value);

    return true;
}

bool TransmissionData::set(const string &key, int value){
    if(!isMember(key))
        return false;

    m_map[key] = valueToString(value);

    return true;
}

bool TransmissionData::set(const string &key, uint value){
    if(!isMember(key))
        return false;

    m_map[key] = valueToString(value);

    return true;
}


int TransmissionData::getInt(const std::string& key) const {
    if(!isMember(key))
        return 0;

    const string& value = m_map.at(key);
    int ret = 0;

	CHECK_STATE_EXCEPTION(value.size() != 0);
	
	bool isNegative = (value[0] == '-');

    for(string::size_type i = (isNegative ? 1 : 0);i<value.size();++i){
    	CHECK_OPERATION_EXCEPTION(value[i] >= '0' && value[i] <= '9');
        ret = ret * 10 + value[i] - '0';
    }

    if(isNegative)
        ret = -ret;

    return ret;
}

typename TransmissionData::uint TransmissionData::getUInt(const std::string& key) const {
    if(!isMember(key))
        return 0;

    const string& value = m_map.at(key);
    int ret = 0;

    for(string::size_type i = 0;i<value.size();++i){
    	CHECK_OPERATION_EXCEPTION(value[i] >= '0' && value[i] <= '9');
        ret = ret * 10 + value[i] - '0';
    }

    return ret;
}

double TransmissionData::getDouble(const std::string& key) const {
    if(!isMember(key))
        return 0.0;

    const string& value = m_map.at(key);
    double ret = 0.0;
    
    int count = sscanf(value.c_str(), "%lf", &ret);

	CHECK_OPERATION_EXCEPTION(count == 1);

    return ret;
}

bool TransmissionData::getBool(const std::string& key) const {
    if(!isMember(key))
        return false;

    CHECK_OPERATION_EXCEPTION( m_map.at(key) == "true" || m_map.at(key) == "false");

    return ( m_map.at(key) == "true" ? true : false );
}

std::string TransmissionData::getString(const std::string& key) const {
    if(!isMember(key))
        return "";

    return m_map.at(key);
}

string& TransmissionData::operator[](const string& key){
    return m_map.at(key);
}

const string& TransmissionData::operator[](const string& key) const {
    return m_map.at(key);
}

bool TransmissionData::remove(const string &key){
    if(!isMember(key)){
        return false;
    }else{
        m_map.erase(key);
        return true;
    }
}

typename TransmissionData::Members TransmissionData::getMembers() const {
    Members members;

    for(Objects::const_iterator cit = m_map.begin();cit!=m_map.end();++cit)
        members.push_back(cit->first);

    return members;
}

bool TransmissionData::isMember(const std::string &key) const {
    return m_map.find(key) != m_map.end();
}
