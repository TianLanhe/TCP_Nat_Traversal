#ifndef SOCKET_H
#define SOCKET_H

#include "../Object.h"

#include <string>

LIB_BEGIN

#define DEFAULT_READ_BYTE 512

class Socket : public Object
{
public:
    Socket(){ }

	virtual bool open() = 0;
	virtual bool close() = 0;
    virtual bool reopen()
    {
        if(isOpen())
            return close() && open();
        else
            return open();
    }

	virtual bool isOpen() const = 0;

    virtual bool bind(const ip_type&, port_type) = 0;	// �ͻ��˷����������԰󶨵�ַ�������ͻ��˿�����ϵͳ���䲻����ʽ����
    virtual bool bind(port_type) = 0;                       // ��������ʱ���Բ�ָ����ַ�����з�����������ipָ��port�İ������յ�
	virtual bool isBound() const = 0;

    virtual std::string read(int readbyte = DEFAULT_READ_BYTE) = 0;
    virtual size_t write(const std::string& str) { return write(str.c_str()); }
    virtual size_t write(const char*) = 0;

    // buffer
    virtual size_t read(char*,int) = 0;
    virtual size_t write(const char*,size_t) = 0;

	virtual port_type getPort() const = 0;
    virtual ip_type getAddr() const = 0;

private:					// ��ֹ���ƺͿ����׽���
	Socket(const Socket&);
	Socket& operator=(const Socket&);
};

LIB_END

#endif
