#ifndef NAT_CHECKER_COMMON_H
#define NAT_CHECKER_COMMON_H

#include <string>

namespace Lib{

enum map_type{
    UNKNOWN,
    ENDPOINT_INDEPENDENT,
    ADDRESS_DEPENDENT,
    ADDRESS_AND_PORT_DEPENDENT
};

typedef map_type filter_type;

class nat_type
{
public:
    nat_type():m_haveNat(false),m_map(UNKNOWN),m_filter(UNKNOWN){ }
    nat_type(bool haveNat,map_type map = (map_type)(0),filter_type filter = (filter_type)(0)):m_haveNat(haveNat),m_map(map),m_filter(filter){ }

    void setNatType(bool haveNat,map_type map = (map_type)(0),filter_type filter = (map_type)(0)){ m_haveNat = haveNat, m_map = map, m_filter = filter; }

    bool haveNat(){ return m_haveNat; }

    map_type getMapType(){ return m_map; }
    filter_type getFilterType(){ return m_filter; }

    std::string getStringMapType(){
        static const char *m_str[4] = { "UNKNOWN", "ENDPOINT_INDEPENDENT", "ADDRESS_DEPENDENT", "ADDRESS_AND_PORT_DEPENDENT" };
        return m_str[m_map];
    }

    std::string getStringFilterType(){
        static const char *m_str[4] = { "UNKNOWN", "ENDPOINT_INDEPENDENT", "ADDRESS_DEPENDENT", "ADDRESS_AND_PORT_DEPENDENT" };
        return m_str[m_filter];
    }

    bool isFullCone(){ return m_map == ENDPOINT_INDEPENDENT && m_filter == ENDPOINT_INDEPENDENT; }
    bool isRestricted(){ return m_map == ENDPOINT_INDEPENDENT && m_filter == ADDRESS_DEPENDENT; }
    bool isPortRestricted(){ return m_map == ENDPOINT_INDEPENDENT && m_filter == ADDRESS_AND_PORT_DEPENDENT; }
    bool isSymmetric(){ return ( m_map == ADDRESS_DEPENDENT || m_map == ADDRESS_AND_PORT_DEPENDENT ) &&
                               ( m_filter == ADDRESS_DEPENDENT || m_map == ADDRESS_AND_PORT_DEPENDENT ); }

private:
    bool m_haveNat;
    map_type m_map;
    filter_type m_filter;
};

#define LOCAL_IP "localIp"
#define LOCAL_PORT "localPort"

#define EXTERN_IP "extIp"
#define EXTERN_PORT "extPort"

#define CHANGE_IP "changeIp"
#define CHANGE_PORT "changePort"

#define CONTINUE "continue"

#define FILTER_TYPE "filter"
#define MAP_TYPE "map"

}

#endif // NAT_CHECKER_COMMON_H
