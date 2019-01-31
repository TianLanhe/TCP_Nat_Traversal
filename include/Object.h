#ifndef OBJECT_H
#define OBJECT_H

#include "Exception.h"

#include <string>

#define LIB_BEGIN namespace Lib {
#define LIB_END }

LIB_BEGIN

#define CHECK_INDEX_OUT_OF_BOUNDS(cond) \
	if(!(cond))							\
		THROW_EXCEPTION(IndexOutOfBoundsException,"Index Out Of Bounds")

#define CHECK_NO_MEMORY_EXCEPTION(pointer)	\
	if((pointer) == NULL)				\
		THROW_EXCEPTION(NoMemoryException,"No Enough Memory To Create Object")

#define CHECK_PARAMETER_EXCEPTION(cond)	\
	if(!(cond))							\
		THROW_EXCEPTION(InvalidParameterException,"Paramter Is Invalid")

#define CHECK_STATE_EXCEPTION(cond)	\
	if(!(cond))						\
		THROW_EXCEPTION(ErrorStateException,"Abnormal State Occurs")

#define CHECK_OPERATION_EXCEPTION(cond)	\
	if(!(cond))							\
		THROW_EXCEPTION(InvalidOperationException,"Operation Is Invalid")

#define MAX_PORT ((unsigned short)(-1))

class Object {
public:
	typedef unsigned short port_type;
	typedef std::string ip_type;
public:
	virtual ~Object(){ }

};

LIB_END

#endif // !OBJECT_H
