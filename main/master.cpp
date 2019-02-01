#include "natchecker/MultNatCheckerServerMaster.h"
#include <iostream>
#include "include/database/DataBase.h"

int main(int argc,char *argv[]){
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " SlaveIp" << std::endl;
        return 1;
    }

    Lib::MultNatCheckerServerMaster check_server;

    check_server.setSlave(argv[1]);
    check_server.setDataBase(new DefaultDataBase<DataRecord>());

    check_server.waitForClient();

	return 0;
}

