#include "include/nattraversal/NatTraversalServer.h"
#include "natchecker/MultNatCheckerServerMaster.h"
#include "include/Log.h"

using namespace Lib;
using namespace std;

int main(int argc,char *argv[]){
    if(argc < 3){
        Log(ERROR) << "Usage: " << argv[0] << " <MasterCheckerServerIP> <SlaveCheckerServerIP>" << eol;
        return 1;
    }

    MultNatCheckerServerMaster check_server;
    check_server.setSlave(argv[2]);

    NatTraversalServer server;

    if(server.init(argv[1],&check_server))
		server.waitForClient();
    else
        Log(ERROR) << "server init error" << eol;

	return 0;
}

