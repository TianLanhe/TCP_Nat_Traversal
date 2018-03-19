#include "../include/transmission/TransmissionData.h"
#include <iostream>
#include "../include/json/json.h"

using namespace std;
using namespace Lib;
using namespace Json;

int main(){
	TransmissionData data;
	data.add("int",3);
	data.add("negative int",-12345);
	data.add("double",123.45);

	
    Value value(Json::objectValue);

    TransmissionData::Members members = data.getMembers();
    for(TransmissionData::Members::size_type i = 0;i<members.size();++i)
        value[members[i]] = data[membrs[i]];

    FastWriter writer;
    string strData = writer.write(value);

	cout << strData << endl;

	return 0;
}

