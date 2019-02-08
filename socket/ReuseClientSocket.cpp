#include "ReuseClientSocket.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#elif defined(linux) || defined(__APPLE__)
#include <sys/socket.h>
#endif

using namespace Lib;

bool ReuseClientSocket::setReuse() {
	int reuse = 1;
	return (setsockopt(m_socket._socket(), SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == 0);
}
