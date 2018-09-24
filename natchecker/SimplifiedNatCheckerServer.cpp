#include "../natchecker/SimplifiedNatCheckerServer.h"
#include "../include/socket/ClientSocket.h"
#include "../socket/DefaultClientSocket.h"
#include "../socket/DefaultServerSocket.h"
#include "../include/socket/ReuseSocketFactory.h"
#include "../include/transmission/TransmissionData.h"
#include "../include/transmission/TransmissionProxy.h"
#include "../include/database/DataBase.h"
#include "../include/Log.h"

#include <sys/select.h>
#include <errno.h>

#include <thread>
#include <string>

extern int errno;

using namespace std;
using namespace Lib;

typedef nat_type::filter_type filter_type;
typedef nat_type::map_type map_type;

SimplifiedNatCheckerServer::SimplifiedNatCheckerServer(const ip_type& main_addr, port_type main_port, port_type another_port)
    :m_main_addr(main_addr),m_main_port(main_port),m_another_port(another_port){
    m_main_server = ReuseSocketFactory::GetInstance()->GetServerSocket();
}

SimplifiedNatCheckerServer::~SimplifiedNatCheckerServer(){
    if(m_main_server)
        delete m_main_server;

    for(vector<DefaultClientSocket>::size_type i=0;i<m_clientVec.size();++i)
        delete m_clientVec[i];
}

bool SimplifiedNatCheckerServer::setListenNum(size_t num){
    if(!m_main_server)
        return false;

    if(!m_main_server->isBound() && !m_main_server->bind(m_main_addr,m_main_port))
        return false;

    return !m_main_server->isListen() && m_main_server->listen(num);
}

void SimplifiedNatCheckerServer::handle_request(ClientSocket* client){
    TransmissionProxy proxy(client);
    TransmissionData data = proxy.read();

    // ��ȡ�� Client ���͹����ı�����ַ���Ա���߹۲쵽���ⲿ��ַ������ⲿ��ַ�� Client ���͹��������Ǳ߹۲쵽�ĵ�ַ��ͬ��
    // ��ʾ Client ���ڹ����У�����ֹͣ NAT ���ͼ�⡣�������ͬ�����Ƚ��� NAT �� Filter ���ͼ��(Client�Ǳ��Ѿ������˼������ȴ�����)��
    // Filter ��������̼��¡������ɺ󣬽���Ҫ Client ���̽�� NAT �� MAP �����Լ��˿�ӳ����ɣ������ϵ��� Client ���ӷ�����Ҫ�����ӵĶ˿�
    // ��������������ӹ������ⲿ��ַ��������ȷ���ͼ����ɣ������� Client �����¶˿ڣ�ֱ�� STUN ������̽�����Ҫ���ˣ��ͷ���ֹͣ̽�����Ϣ
    // �������ɽ��������̻�õ�����Ϣ�洢�����Ա�������;

    if(!data.isMember(LOCAL_IP) || !data.isMember(LOCAL_PORT) || !data.isMember(IDENTIFIER))
        return;

    ip_type local_ip = data.getString(LOCAL_IP);
    port_type local_port = data.getInt(LOCAL_PORT);

    ip_type ext_ip = client->getPeerAddr();
    port_type ext_port = client->getPeerPort();

    DataRecord record;
    record.setIdentifier(data.getString(IDENTIFIER));

    Address addr(local_ip,local_port);
    record.setLocalAddress(addr);

    addr.ip = ext_ip;
    addr.port = ext_port;
    record.setExtAddress(addr);

    Log(INFO) << "local_ip: " << local_ip << eol;
    Log(INFO) << "local_port: " << local_port << eol;

    Log(INFO) << "ext_ip: " << ext_ip << eol;
    Log(INFO) << "ext_port: " << ext_port << eol;

    // ���Ǻ���ֵ�����IP ��ͬ�˿�ȴ�ı���
    if(local_ip == ext_ip && local_port != ext_port){
        Log(WARN) << "local IP is equal to extern IP but local port is different from extern port." << eol;
    }

    if(local_ip == ext_ip && local_port == ext_port)	// �����ַ��ͬ�����ڹ����У�ֹͣ���
    {
        data.clear();
        data.add(CONTINUE,false);
        proxy.write(data);

        record.setNatType(nat_type(false));
    }
    else
    {   // �����ַ��ͬ������ NAT������ NAT �� Filter ���ͼ�� �� �򻯰�� Fileter ���ͼ��ֻ�ܸı�˿����ӣ�ֻ���ж��Ƿ��� Address And Port Denpendent
        // ������ı�˿��������ϣ���˵�������� EndPoint Independent �� Address Dependent �� Filter ���򣬵��޷���һ��ȷ�����������Ϊ�� Address Dependent��
        ClientSocket *c = ReuseSocketFactory::GetInstance()->GetClientSocket();
        if(!c->bind(m_main_addr,m_another_port)){
            delete c;
            return;
        }

        bool canConnect = c->connect(ext_ip,ext_port,_getConnectRetryTime());
        filter_type filterType;
        if(canConnect)
        {
            Log(INFO) << m_main_addr << ":" << m_another_port << " connect to " << ext_ip << ":" << ext_port << " successfully" << eol;
            Log(INFO) << "filterType: ADDRESS_DEPENDENT" << eol;
            filterType = nat_type::ADDRESS_DEPENDENT;
        }
        else
        {
            Log(INFO) << m_main_addr << ":" << m_another_port << " fail to connect to " << ext_ip << ":" << ext_port << eol;
            Log(INFO) << "filterType: ADDRESS_AND_PORT_DEPENDENT" << eol;
            filterType = nat_type::ADDRESS_AND_PORT_DEPENDENT;
        }

        delete c;
        c = NULL;

        // ͬ���ģ��򻯰�� MAP ���ͼ�������޷����� IP�����ֻ������ PD �� ��PD ���� NAT �� Map ����Ϊ ��PD������Ϊ�� AD
        // ���ȷ������� IP1:Prot2 ������֪ͨ Client ���ӣ�����һ��(��2��)���۲쵽�ĵ�ַ����һ��(��1��)��ͬ�����ʾ NAT ���� EndPoint Independent ���� EndPoint Independent �� Map ����
        // ����Ϊ NAT �� Map ����Ϊ AD����ʱ������˿�Ϊ Extern_Port - 1��Ԥ������Ϊ1
        // ����2�����1�ι۲쵽�ĵ�ַ��ͬ����ʾ NAT �� Map ����Ϊ PD����ʱ����֪ͨ Client ����������Ԥ������

        ServerSocket *s = ReuseSocketFactory::GetInstance()->GetServerSocket();
        if(!(s->bind(m_main_addr,m_another_port) &&
             s->listen(DEFAULT_LISTEN_NUM))){
            delete s;
            return;
        }

        data.clear();
        data.add(CONTINUE,true);
        data.add(FILTER_TYPE,filterType);
        data.add(EXTERN_IP,ext_ip);
        data.add(EXTERN_PORT,ext_port);
        data.add(CHANGE_IP,m_main_addr);
        data.add(CHANGE_PORT,m_another_port);

        proxy.write(data);		// STUN ����������Ϣ���͸� Client�����ȴ� Client ����

        c = s->accept();

        string ext_ip2 = c->getPeerAddr();
        port_type ext_port2 = c->getPeerPort();

        Log(INFO) << "ext_ip2: ",ext_ip2 << eol;
        Log(INFO) << "ext_port2: ",ext_port2 << eol;

        // ���� NAT ֻ��һ������ IP �� ͬһ��������������ͨ��ʱ�϶���ת����ͬһ�� IP (Ҳ���ı�˿�)
        if(ext_ip2 != ext_ip)
            Log(WARN) << "The NAT allocate the different global IP to the same host" << eol;

        if(ext_ip2 == ext_ip && ext_port2 == ext_port) // ��2�ε�������ַ���1�ε���ͬ
        {
            data.clear();
            data.add(MAP_TYPE,nat_type::ADDRESS_DEPENDENT);
            data.add(CONTINUE,false);

            proxy.setSocket(c);
            proxy.write(data);

            delete c;
            c = NULL;

            // ���ö˿�����Ϊ1
            nat_type natType(true,nat_type::ADDRESS_DEPENDENT,filterType);
            natType.setPrediction(true,1);
            record.setNatType(natType);

            Log(INFO) << "mapType: ADDRESS_DEPENDENT" << eol;
        }
        else	// ��2�ε�������ַ���1�εĲ�ͬ������ Address And Port Dependent�����ж������Ԥ������
        {
            int delta_pre = ext_port2 - ext_port - 1;
            int delta_cur = ext_port2 - ext_port;
            size_t try_time = 0;
            port_type ext_port_pre = ext_port2;

            ip_type ext_ip_n;
            port_type ext_port_n;

            while( (delta_pre != delta_cur || delta_cur + ext_port_pre >= 65535) && try_time++ < _getMaxTryTime()){
                data.clear();
                data.add(CONTINUE,true);
                data.add(CHANGE_IP,m_main_addr);
                data.add(CHANGE_PORT,m_another_port + (port_type)(try_time));

                if(!(s->close() && s->open() &&
                        s->bind(m_main_addr,m_another_port + try_time) &&
                        s->listen(DEFAULT_LISTEN_NUM)))
                    return;

                proxy.setSocket(c);
                proxy.write(data);

                if(c != NULL)
                    delete c;
                c = s->accept();

                ext_ip_n = c->getPeerAddr();
                ext_port_n = c->getPeerPort();

                Log(INFO) << "ext_ip_n: " << ext_ip_n << eol;
                Log(INFO) << "ext_port_n: " << ext_port_n << eol;

                delta_pre = delta_cur;
                delta_cur = ext_port_n - ext_port_pre;
                ext_port_pre = ext_port_n;
            }

            data.clear();
            data.add(MAP_TYPE,nat_type::ADDRESS_AND_PORT_DEPENDENT);
            data.add(CONTINUE,false);

            proxy.setSocket(c);
            proxy.write(data);

            if(c != NULL)
                delete c;
            c = NULL;

            nat_type natType(true,nat_type::ADDRESS_AND_PORT_DEPENDENT,filterType);
            Log(INFO) << "mapType: ADDRESS_AND_PORT_DEPENDENT" << eol;
            if( try_time == _getMaxTryTime() ){ // �˿�����仯
                natType.setPrediction(false);
            }else{  // ��������Ϊ delta_cur;
                natType.setPrediction(true,delta_cur);
            }
            record.setNatType(natType);
            record.setExtAddress(Address(ext_ip_n,ext_port_n + delta_cur));	// �ǵø��� DataRecord ���ⲿ��ַ�������µ�Ϊ׼
        }
        delete s;
        s = NULL;
    }
    //delete client;		// ע�����ﲻ�ܰ� STUN �������� client �������ĵ�һ�����Ӹ��ϵ�����Ϊ��Щ�Գ��� NAT ��һ����������������Դ�˿�
    //client = NULL;			���ڶ��ο�ʼ�Ŵ�һ�����ֵ���е������䣬��������һ�����Ӷϵ��ˣ�NAT ���ܻḴ������˿ڣ��˿ڲ²����Ч

    CHECK_STATE_EXCEPTION(m_database->addRecord(record));
}

void SimplifiedNatCheckerServer::waitForClient(){
    if(!m_main_server->isListen() && !setListenNum(DEFAULT_LISTEN_NUM))
        THROW_EXCEPTION(InvalidOperationException,"bind or listen error");

    /*ClientSocket *client;
    while(client = m_main_server->accept()){
        handle_request(client);			// Ŀǰ�Ĵ����ǲ�Ҫ����ִ�У���һ������һ������Ϊ��� NAT ���͵Ĺ����л�ȴ��Է��������ӣ����������죬Ҳ������߳̽��ܵ��� socket ��������һ���߳���Ҫ��
    }*/
    // ���ڲ�������ɾ�˵�һ�����ӵ� client����������� I/O ��·ת�ӵķ�ʽ���ܹ������� client �Ͽ����ӵ�������ɿͻ��������Ͽ�

    DefaultServerSocket *m_socket = dynamic_cast<DefaultServerSocket*>(m_main_server);
    CHECK_STATE_EXCEPTION(m_socket);

    fd_set set;
    int ret,maxfd,fd;
    while(1)
    {
        FD_ZERO(&set);

        FD_SET(m_socket->_getfd(),&set);		// �ѷ����� Socket �����н��ܵ��Ŀͻ������� Socket ����ӵ� set ��
        maxfd = m_socket->_getfd();

        for(vector<DefaultClientSocket*>::size_type i=0;i<m_clientVec.size();++i){
            fd = m_clientVec[i]->_getfd();

            FD_SET(fd,&set);

            if(maxfd < fd)
                maxfd = fd;
        }

        ret = select(maxfd+1,&set,NULL,NULL,NULL);

        if(ret == -1)
        {
            if(errno == EINTR)
                Log(WARN) << "NatCheckerServer is interrupted at function select" << eol;
            else
                THROW_EXCEPTION(ErrorStateException,"select error in NatCheckerServer::waitForClient");
        }else if(ret == 0)
        {
            THROW_EXCEPTION(ErrorStateException,"select timeout in NatTraversalServer::waitForClient");
        }else
        {
            // ����������˿ɶ����༴���¿ͻ����������ӣ���������ӣ�Ϊ����񣬽��� NAT ���ͼ��
            if(FD_ISSET(m_socket->_getfd(),&set))
            {
                ClientSocket *client = m_socket->accept();

                Log(INFO) << "Client \"" << client->getPeerAddr() << ':' << client->getPeerPort() << "\" connect" << eol;

                DefaultClientSocket *defaultSocket = dynamic_cast<DefaultClientSocket*>(client);
                CHECK_STATE_EXCEPTION(defaultSocket);

                m_clientVec.push_back(defaultSocket);

                handle_request(client);

                --ret;
            }

            // �����洢�Ŀͻ��� socket �ж��Ƿ�ɶ������ɶ��ͻ��˶Ͽ�����
            vector<DefaultClientSocket*>::iterator it = m_clientVec.begin();
            while(it != m_clientVec.end() && ret != 0){
                if(FD_ISSET((*it)->_getfd(),&set))
                {
                    string content = (*it)->read();
                    CHECK_STATE_EXCEPTION(content.empty());

                    Log(INFO) << "Client \"" << (*it)->getPeerAddr() << ':' << (*it)->getPeerPort() << "\" disconnect" << eol;

                    delete (*it);

                    --ret;
                    it = m_clientVec.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
}
