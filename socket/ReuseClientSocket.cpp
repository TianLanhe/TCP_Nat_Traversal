#include "ReuseClientSocket.h"
#include <sys/socket.h>

using namespace Lib;

bool ReuseClientSocket::setReuse() {
	int reuse = 1;
	return (setsockopt(m_socket._socket(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == 0);
}
