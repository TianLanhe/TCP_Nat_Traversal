Object = DefaultClientSocket.o \
	 DefaultServerSocket.o \
	 DefaultSocketFactory.o \
	 DefaultSocket.o \
	 Exception.o
	 
InDir = include/socket

icf_exception = 
icf_object = include/Exception.h $(icf_exception)
icf_socket = include/Object.h $(icf_object)
icf_client_socket = $(InDir)/Socket.h $(icf_socket)
icf_server_socket = $(InDir)/Socket.h $(icf_socket)
icf_default_socket = include/Object.h $(icf_object)
icf_default_client_socket = socket/DefaultSocket.h $(icf_default_socket)  $(InDir)/ClientSocket.h $(icf_client_socket)
icf_default_server_socket = socket/DefaultSocket.h $(icf_default_socket)  $(InDir)/ServerSocket.h $(icf_server_socket)
icf_default_factory = include/Object.h $(icf_object)
icf_reuse_factory = include/Object.h $(icf_object)
icf_reuse_client_socket = socket/DefaultClientSocket.h $(icf_default_client_socket)
icf_reuse_server_socket = socket/DefaultServerSocket.h $(icf_default_server_socket)

client: client.o $(Object)
	g++ -o client client.o $(Object)
	
server: server.o $(Object)
	g++ -o server server.o $(Object)
	
client.o: client.cpp \
	$(InDir)/DefaultSocketFactory.h \
	$(InDir)/ClientSocket.h \
	$(InDir)/Socket.h \
	include/Object.h \
	include/Exception.h
	g++ -c client.cpp
	
server.o: server.cpp \
	$(InDir)/DefaultSocketFactory.h \
	$(InDir)/ServerSocket.h \
	$(InDir)/Socket.h \
	include/Object.h \
	include/Exception.h
	g++ -c server.cpp
	
DefaultSocketFactory.o: $(InDir)/DefaultSocketFactory.h \
				socket/DefaultSocketFactory.cpp \
				socket/DefaultServerSocket.h \
				socket/DefaultClientSocket.h \
				$(icf_default_client_socket) \
				$(icf_default_server_socket)
	g++ -c socket/DefaultSocketFactory.cpp
	
DefaultClientSocket.o: socket/DefaultClientSocket.h \
				socket/DefaultClientSocket.cpp \
				$(icf_default_client_socket)
	g++ -c socket/DefaultClientSocket.cpp
	
DefaultServerSocket.o: socket/DefaultServerSocket.h \
				socket/DefaultServerSocket.cpp \
				$(icf_default_server_socket) \
				$(icf_default_client_socket)
	g++ -c socket/DefaultServerSocket.cpp
	
Exception.o:include/Exception.h source/Exception.cpp
	g++ -c source/Exception.cpp
	
DefaultSocket.o: socket/DefaultSocket.h socket/DefaultSocket.cpp $(icf_default_socket)
	g++ -c socket/DefaultSocket.cpp

clean:
	rm -rf *.o

