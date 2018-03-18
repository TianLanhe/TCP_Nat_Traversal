#ifndef SOCKET_H
#define SOCKET_H

#include "../Object.h"

#include <string>

LIB_BEGIN

class Socket : public Object {
public:
	typedef unsigned short port_type;

public:
	Socket() {}

	virtual bool open() = 0;
	virtual bool close() = 0;

	virtual bool isOpen() const = 0;

	virtual bool bind(port_type, const std::string&) = 0;	// �ͻ��˷����������԰󶨵�ַ�������ͻ��˿�����ϵͳ���䲻����ʽ����
	virtual bool isBound() const = 0;

	virtual std::string read(int) = 0;
	virtual size_t write(const std::string& str) { return write(str.c_str()); };
	virtual size_t write(const char*) = 0;

	virtual port_type getPort() const = 0;
	virtual std::string getAddr() const = 0;

private:					// ��ֹ���ƺͿ����׽���
	Socket(const Socket&);
	Socket& operator=(const Socket&);
};

LIB_END

#endif
