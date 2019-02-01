#include "SocketReadSelector.h"
#include <sys/select.h>

using namespace Lib;

int SocketReadSelector::_select(int maxfd,fd_set* set,struct timeval* ptrto){
    return  ::select(maxfd+1,set,NULL,NULL,ptrto);
}
