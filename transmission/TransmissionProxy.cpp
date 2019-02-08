#include "../include/transmission/TransmissionProxy.h"
#include "../include/json/json.h"
#include "../include/socket/ClientSocket.h"
#include "../include/transmission/TransmissionData.h"

#include <string>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#elif defined(linux) || defined(__APPLE__)
#include <arpa/inet.h>
#endif

using namespace std;
using namespace Lib;
using namespace Json;

TransmissionProxy::TransmissionProxy():m_socket(NULL){
}

TransmissionProxy::TransmissionProxy(ClientSocket *client){
    setSocket(client);
}

bool TransmissionProxy::setSocket(ClientSocket *socket){
    CHECK_PARAMETER_EXCEPTION(socket);

    if(!socket->isConnected())
        return false;

    m_socket = socket;

    return true;
}

bool TransmissionProxy::write(const TransmissionData & data){
    CHECK_OPERATION_EXCEPTION(m_socket);

    Value value(Json::objectValue);

    TransmissionData::Members members = data.getMembers();
    for(TransmissionData::Members::size_type i = 0;i<members.size();++i)
        value[members[i]] = data[members[i]];

    FastWriter writer;
    string strData = writer.write(value);

    uint32_t bytes = htonl(strData.size());
    char dataLen[6] = { 0 };
    memcpy(dataLen+1,&bytes,4);

    int count = 0;
    for(int i=4;i>=1;--i){
        if(dataLen[i] == 0)
            break;
        ++count;
    }

    dataLen[0] = count;
    int p = 1;
    for(int i=0;i<=count;++i)
        dataLen[p++] = dataLen[5-count+i];

    string content(dataLen);
    content.append(strData);

    size_t len = m_socket->write(content);
    if(len != content.size())
        return false;

    return true;
}

TransmissionData TransmissionProxy::read(){
    CHECK_OPERATION_EXCEPTION(m_socket);

    TransmissionData ret;

    string strBytes = m_socket->read(1);
    if(strBytes.size() != 1)
        return ret;

    size_t lengthOfSize = strBytes[0];
    strBytes = m_socket->read(lengthOfSize);
    if(strBytes.size() != lengthOfSize)
        return ret;

    strBytes.insert(0,4-lengthOfSize,'\0');

    uint32_t intBytes;
    memcpy(&intBytes,strBytes.c_str(),4);
    size_t bytes = ntohl(intBytes);

    string content = m_socket->read(bytes);
    if(content.size() != bytes)
        return ret;

    Reader reader;
    Value value;
    reader.parse(content,value);

    Value::Members members = value.getMemberNames();
    for(Value::Members::size_type i=0;i<members.size();++i)
        ret.add(members[i],value[members[i]].asString());

    return ret;
}
