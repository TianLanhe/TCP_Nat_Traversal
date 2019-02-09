#include "SocketWriteSelector.h"

using namespace Lib;

int SocketWriteSelector::_select(int maxfd,fd_set* set,struct timeval* ptrto){
    return  ::select(maxfd+1,NULL,set,NULL,ptrto);
}
