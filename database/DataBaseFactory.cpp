#include "../include/database/DataBaseFactory.h"
#include "DefaultDataBase.h"

using namespace Lib;

DataBaseFactory* DataBaseFactory::factory = NULL;

DataBase* DataBaseFactory::GetDataBase(){
	DataBase *ret = new DefaultDataBase();
	CHECK_NO_MEMORY_EXCEPTION(ret);
	return ret;
}

DataBaseFactory* DataBaseFactory::GetInstance(){
	if(!factory){
		factory = new DataBaseFactory();
		CHECK_NO_MEMORY_EXCEPTION(factory);
	}
	return factory;
}

