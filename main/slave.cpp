#include "natchecker/MultNatCheckerServerSlave.h"
#include "include/Log.h"

using namespace Lib;

int main(){
    MultNatCheckerServerSlave check_server;

    check_server.waitForClient();

	return 0;
}

