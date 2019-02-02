#include "ListenDirectlyCommand.h"
#include "include/socket/ClientSocket.h"
#include "include/socket/ReuseSocketFactory.h"
#include "include/socket/ServerSocket.h"
#include "include/SmartPointer.h"
#include "include/socket/SocketSelector.h"
#include "include/socket/SocketSelectorFactory.h"

#include <vector>

#include <sys/select.h>

using namespace Lib;
using namespace std;

ClientSocket* ListenDirectlyCommand::traverse(const TransmissionData &data, const ip_type &ip, port_type port)
{
    if(data.getInt(TYPE) != LISTEN_DIRECTLY)
        return NULL;

    SmartPointer<ServerSocket> server(ReuseSocketFactory::GetInstance()->GetServerSocket());

    if(!server->bind(ip,port) || !server->listen(LISTEN_NUMBER))
        return NULL;

    SmartPointer<SocketSelector> selector(SocketSelectorFactory::GetInstance()->GetReadSelector());
    selector->add(server.get());

    vector<Socket*> vecSockets = selector->select(SELECT_WAIT_TIME_DOUBLE);
    if(vecSockets.empty())
        return NULL;
    else{
        CHECK_STATE_EXCEPTION(vecSockets.size() == 1);

        ServerSocket *s = dynamic_cast<ServerSocket*>(vecSockets[0]);

        return s->accept();
    }
}
