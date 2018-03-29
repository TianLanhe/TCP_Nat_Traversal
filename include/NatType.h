#ifndef NAT_TYPE_H
#define NAT_TYPE_H

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
	typedef unsigned short port_type;
	
public:
    nat_type():m_isPredictable(true),m_port_delta(0),m_haveNat(false),m_map(UNKNOWN),m_filter(UNKNOWN){ }
    nat_type(bool haveNat,map_type map = UNKNOWN,filter_type filter = UNKNOWN):m_haveNat(haveNat),m_map(map),m_filter(filter),m_isPredictable(true),m_port_delta(0){ }

    void setNatType(bool haveNat,map_type map = UNKNOWN,filter_type filter = UNKNOWN){ m_haveNat = haveNat, m_map = map, m_filter = filter; }

	void setPrediction(bool isPredictable = true, port_type delta = 0){ m_isPredictable = isPredictable, m_port_delta = delta; }

    bool haveNat() const { return m_haveNat; }
    bool isPredictable() const { return m_isPredictable; }

    map_type getMapType() const { return m_map; }
    filter_type getFilterType() const { return m_filter; }
    port_type getPortDelta() const { return m_port_delta; }

    std::string getStringMapType() const {
        static const char *m_str[4] = { "UNKNOWN", "ENDPOINT_INDEPENDENT", "ADDRESS_DEPENDENT", "ADDRESS_AND_PORT_DEPENDENT" };
        return m_str[m_map];
    }

    std::string getStringFilterType() const {
        static const char *m_str[4] = { "UNKNOWN", "ENDPOINT_INDEPENDENT", "ADDRESS_DEPENDENT", "ADDRESS_AND_PORT_DEPENDENT" };
        return m_str[m_filter];
    }

    bool isFullCone() const { return m_map == ENDPOINT_INDEPENDENT && m_filter == ENDPOINT_INDEPENDENT; }
    bool isRestricted() const { return m_map == ENDPOINT_INDEPENDENT && m_filter == ADDRESS_DEPENDENT; }
    bool isPortRestricted() const { return m_map == ENDPOINT_INDEPENDENT && m_filter == ADDRESS_AND_PORT_DEPENDENT; }
    bool isSymmetric() const { return ( m_map == ADDRESS_DEPENDENT || m_map == ADDRESS_AND_PORT_DEPENDENT ) &&
                               ( m_filter == ADDRESS_DEPENDENT || m_map == ADDRESS_AND_PORT_DEPENDENT ); }

private:
	bool m_isPredictable;			// Review: 目前设计为客户端不知道 NAT 是否可预测以及预测增量是多少，只有服务器端知道这两个信息
	port_type m_port_delta;			// 客户端根据服务器的指令打洞或连接或监听，不需要知道 NAT 是否的这两个信息
	
    bool m_haveNat;
    map_type m_map;
    filter_type m_filter;
};

}

#endif // !NAT_TYPE_H

