#ifndef NAT_TRAVERSAL_SERVER_H
#define NAT_TRAVERSAL_SERVER_H

#include "../Object.h"
#include "../observer/Observer.h"
#include "../Semaphore.h"

#include <map>
#include <vector>

LIB_BEGIN

class DefaultServerSocket;
class DefaultClientSocket;
class NatCheckerServer;
class DataRecord;

template < typename T >
class DefaultDataBase;

#define SERVER_PORT 9999
#define STUN_MAIN_PORT 6666
#define STUN_SECONDARY_PORT 8888

class UserRecord : public Object
{
public:
    UserRecord():m_isReady(false),m_socket(NULL){ }
    UserRecord(const std::string& identifier,DefaultClientSocket *socket):m_identifier(identifier),m_isReady(false),m_socket(socket){ }

    std::string getIdentifier() const { return m_identifier; }
    bool isReady() const { return m_isReady; }
    DefaultClientSocket* getClientSocket() const { return m_socket; }

    void setIdentifier(const std::string& identifier) { m_identifier = identifier; }
    void setReady(bool isReady) { m_isReady = isReady; }
    void setClientSocket(DefaultClientSocket* socket) { m_socket = socket; }

private:
    std::string m_identifier;
    DefaultClientSocket *m_socket;
    bool m_isReady;
};

class NatTraversalServer : public Object , public Observer
{
public:
    typedef DefaultDataBase<UserRecord> UserManager;

public:
    NatTraversalServer():m_socket(NULL),m_database(NULL),m_checker_server(NULL),m_user_manager(NULL){ }

    bool init();

    void term();

    // this function will run forever, you should call this function in a new thread
    void waitForClient();

public:
    void notify(void*);

private:
    void getAndStoreIdentifier(DefaultClientSocket*);

    static void handle_connect_request(NatTraversalServer*,const std::string&,DefaultClientSocket*);

    void waitForDataBaseUpdate(const std::string&);

private:
    NatCheckerServer *m_checker_server;
    DefaultServerSocket *m_socket;

    DefaultDataBase<DataRecord> *m_database;

    ip_type m_main_ip;
    ip_type m_scondary_ip;

    UserManager* m_user_manager;

    std::map<std::string,Semaphore> m_semaphore;
};

LIB_END

#endif // !NAT_TRAVERSAL_SERVER_H
