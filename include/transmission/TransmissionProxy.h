#ifndef TRANSMISSION_PROXY_H
#define TRANSMISSION_PROXY_H

#include "../Object.h"

LIB_BEGIN

class ClientSocket;
class TransmissionData;

class TransmissionProxy : public Object
{
public:
    TransmissionProxy();
    TransmissionProxy(ClientSocket*);

    bool write(const TransmissionData&);
    TransmissionData read();

    bool setSocket(ClientSocket*);

private:
    ClientSocket *m_socket;
};

LIB_END

#endif // TRANSMISSION_PROXY_H
