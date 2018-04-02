#include "../include/nattraversal/NatTraversalServer.h"

using namespace Lib;

int main(){
    NatTraversalServer server;

    if(server.init())
		server.waitForClient();

	return 0;
}

