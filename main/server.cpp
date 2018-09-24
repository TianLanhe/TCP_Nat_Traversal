#include "../include/nattraversal/NatTraversalServer.h"
#include "../natchecker/SimplifiedNatCheckerServer.h"
#include "../include/Log.h"

using namespace Lib;
using namespace std;

int main(){
   /* vector<string> ips = Util::getLocalIps();
    if(ips.size() < 2){
        Log(ERROR) << "there haven't two ip";
        return 1;
    }

    Log(INFO) << "IP: ";
    for(vector<string>::const_iterator cit = ips.begin();cit!=ips.end();++cit){
        if(cit != ips.begin())
            Log(INFO) << ',';
        Log(INFO) << *cit;
    }
    Log(INFO) << eol;

    StandardNatCheckerServer check_server(ips[0],STUN_MAIN_PORT,ips[1],STUN_SECONDARY_PORT);*/

    SimplifiedNatCheckerServer check_server("138.128.197.157",STUN_MAIN_PORT,STUN_SECONDARY_PORT);

    NatTraversalServer server;

    if(server.init("138.128.197.157",&check_server))
		server.waitForClient();
    else
        Log(ERROR) << "server init error" << eol;

	return 0;
}

