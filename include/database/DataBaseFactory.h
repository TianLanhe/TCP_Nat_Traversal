#ifndef DATA_BASE_FACTORY_H
#define DATA_BASE_FACTORY_H

#include "../Object.h"

LIB_BEGIN

class DataBase;

class DataBaseFactory : public Object {
public:
    DataBase* GetDataBase();
	static DataBaseFactory* GetInstance();

private:
	DataBaseFactory() { }
	static DataBaseFactory* factory;
};

LIB_END

#endif // DATA_BASE_FACTORY_H

