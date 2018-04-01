#ifndef NAT_TRAVERSAL_CLIENT_H
#define NAT_TRAVERSAL_CLIENT_H

#include "../Object.h"

#include <string>
#include <mutex>
#include <condition_variable>

LIB_BEGIN

#define CLIENT_DEFAULT_PORT 18899

class ClientSocket;

class NatTraversalClient : public Object
{
public:
    NatTraversalClient(const std::string&);
    ~NatTraversalClient();

    bool enroll(const ip_type&,port_type);
    bool hasEnrolled();
    
    // m_isReady用于指示是否准备可以接收对等方的连接请求，用来同步下面两个函数，故对 m_isReady 的修改必须线程安全
    bool setReadyToAccept(bool ready = true);
    bool isReadyToAccept();

	// 这两个函数用于在两条线程中运行，一条随时准备主动发起与对等端的连接，一条随时监听，准备接受服务器端的信令以被动和对等方发起连接
	// 注意：当主动发起对等端连接时，会暂时屏蔽 waitForPeerHsot，因为此时 Client 要与服务器通信，等主动连接结束后，让出信道让其监听服务器的信令
    ClientSocket* connectToPeerHost(const std::string&);
    ClientSocket* waitForPeerHost();

private:
    bool _setReadySafely(bool);

private:
    ClientSocket *m_socket;		// 这条用于与服务器保持持久连接，传输信令的 TCP 连接，不具有 Reuse 属性

    std::string m_identifier;

    bool m_isReady;
    std::mutex m_mutex;
    std::condition_variable m_conVar;

    bool m_isConnecting;
    std::mutex m_mtx;
    std::condition_variable m_cnv;
};

LIB_END

#endif // !NAT_TRAVERSAL_CLIENT_H
