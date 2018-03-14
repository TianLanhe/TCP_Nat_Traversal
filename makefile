Object = DefaultClientSocket.o \
	 DefaultServerSocket.o \
	 DefaultSocketFactory.o \
	 DefaultSocket.o \
	 Exception.o
	 
InDir = include/socket

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
				$(InDir)/ClientSocket.h \
				$(InDir)/ServerSocket.h \
				$(InDir)/Socket.h \
				socket/DefaultSocket.h \
				include/Object.h \
				include/Exception.h
	g++ -c socket/DefaultSocketFactory.cpp
	
DefaultClientSocket.o: socket/DefaultClientSocket.h \
				socket/DefaultClientSocket.cpp \
				socket/DefaultSocket.h \
				$(InDir)/ClientSocket.h \
				$(InDir)/Socket.h \
				include/Object.h \
				include/Exception.h
	g++ -c socket/DefaultClientSocket.cpp
	
DefaultServerSocket.o: socket/DefaultServerSocket.h \
				socket/DefaultServerSocket.cpp \
				$(InDir)/ServerSocket.h \
				$(InDir)/Socket.h \
				include/Object.h \
				include/Exception.h \
				socket/DefaultSocket.h \
				$(InDir)/ClientSocket.h \
				socket/DefaultClientSocket.h
	g++ -c socket/DefaultServerSocket.cpp
	
Exception.o:include/Exception.h source/Exception.cpp
	g++ -c source/Exception.cpp
	
DefaultSocket.o: socket/DefaultSocket.h socket/DefaultSocket.cpp \
				include/Object.h \
				include/Exception.h
	g++ -c socket/DefaultSocket.cpp

clean:
	rm -rf *.o

