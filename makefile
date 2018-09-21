default_sock_obj = DefaultClientSocket.o \
		DefaultServerSocket.o \
		DefaultSocketFactory.o \
		DefaultSocket.o

reuse_sock_obj = ReuseClientSocket.o \
		ReuseServerSocket.o \
		ReuseSocketFactory.o \
		DefaultClientSocket.o \
		DefaultServerSocket.o \
		DefaultSocket.o

trans_obj = TransmissionProxy.o \
	TransmissionData.o \
	json_reader.o \
	json_writer.o \
	json_value.o
	
server_obj = NatTraversalServer.o \
			NatCheckerServer.o

client_obj = NatTraversalClient.o \
			NatCheckerClient.o
			
traversal_command_obj = TraversalCommand.o \
						ConnectDirectlyCommand.o \
						ConnectRandomlyCommand.o \
						ConnectAroundCommand.o \
						ListenDirectlyCommand.o \
						ListenAndPunchCommand.o \
						ListenAndPunchRandomlyCommand.o
	
########################################## directory
	 
sock_inc_dir = include/socket
json_inc_dir = include/json
trans_inc_dir = include/transmission
nat_traversal_inc_dir = include/nattraversal
nat_checker_inc_dir = include/natchecker
database_inc_dir = include/database
observer_inc_dir = include/observer
traversal_command_inc_dir = include/traversalcommand

########################################## base

icf_log = 
icf_semaphore = 
icf_exception = 
icf_nat_type = 
icf_object = include/Exception.h $(icf_exception)
icf_smart_pointer = include/Exception.h $(icf_exception)
icf_utility =
icf_log = 

########################################## socket

icf_socket = include/Object.h $(icf_object)
icf_client_socket = $(sock_inc_dir)/Socket.h $(icf_socket)
icf_server_socket = $(sock_inc_dir)/Socket.h $(icf_socket)
icf_default_factory = include/Object.h $(icf_object)
icf_reuse_factory = include/Object.h $(icf_object)

icf_default_socket = include/Object.h $(icf_object)
icf_default_client_socket = socket/DefaultSocket.h $(icf_default_socket) \
			$(sock_inc_dir)/ClientSocket.h $(icf_client_socket)
icf_default_server_socket = socket/DefaultSocket.h $(icf_default_socket) \
			$(sock_inc_dir)/ServerSocket.h $(icf_server_socket)

icf_reuse_client_socket = socket/DefaultClientSocket.h $(icf_default_client_socket)
icf_reuse_server_socket = socket/DefaultServerSocket.h $(icf_default_server_socket)

########################################### json

icf_config = 
icf_forwards = json/config.h $(icf_config)
icf_features = json/forwards.h $(icf_forwards)
icf_autolink = json/config.h $(icf_config)

icf_value = json/forwards.h $(icf_forwards)
icf_writer_actually = json/value.h $(icf_value)
icf_reader_actually = json/features.h $(icf_features) \
		json/value.h $(icf_value)
icf_json_actually = json/autolink.h $(icf_autolink) \
		json/reader.h $(icf_reader_actually) \
		json/writer.h $(icf_writer_actually)

icf_json = json/json.h $(icf_json_actually)
icf_reader = json/reader.h $(icf_reader_actually)
icf_writer = json/writer.h $(icf_writer_actually)

########################################### transmission

icf_trans_data = include/Object.h $(icf_object)
icf_trans_proxy = include/Object.h $(icf_object)
		
########################################### nat_checker

icf_nat_checker_common = include/NatType.h $(icf_nat_type)
icf_nat_checker_client = include/Object.h $(icf_object) \
						$(nat_checker_inc_dir)/NatCheckerCommon.h $(icf_nat_checker_common) \
						$(sock_inc_dir)/ClientSocket.h $(icf_client_socket)
icf_nat_checker_server = include/Object.h $(icf_object) \
						$(nat_checker_inc_dir)/NatCheckerCommon.h $(icf_nat_checker_common) \
						$(sock_inc_dir)/ServerSocket.h $(icf_server_socket)

########################################### observer 

icf_observer = 
icf_subject = $(observer_inc_dir)/Observer.h $(icf_observer)
						
########################################### database

icf_database = include/Object.h $(icf_object)
icf_default_database = $(database_inc_dir)/DataBase.h $(icf_databse) \
						$(observer_inc_dir)/Subject.h $(icf_subject)
						
########################################### traversal_command

icf_traversal_command = include/Object.h $(icf_object) \
						include/NatType.h $(icf_nat_type) \
						$(trans_inc_dir)/TransmissionData.h $(icf_trans_data)
icf_connect_directly = $(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command)
icf_connect_around = $(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command)
icf_connect_randomly = $(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command)
icf_listen_directly = $(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command)
icf_listen_and_punch = $(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command)
icf_listen_and_punch_some = $(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command)
						
########################################### nat_traversal

icf_nat_traversal_client = include/Object.h $(icf_object)
icf_nat_traversal_server = include/Object.h $(icf_object) \
							$(observer_inc_dir)/Observer.h $(icf_observer) \
							include/Semaphore.h $(icf_semaphore)
icf_nat_traversal_common = 

########################################### target 

all: client server

test: mytest.o $(trans_obj) $(reuse_sock_obj) $(client_obj) Exception.o
	g++ -o mytest mytest.o $(trans_obj) $(reuse_sock_obj) $(client_obj) Exception.o -lpthread

client: client.o $(trans_obj) $(reuse_sock_obj) $(client_obj) $(traversal_command_obj) Exception.o Utility.o
	g++ -o client client.o $(trans_obj) $(reuse_sock_obj) $(client_obj) $(traversal_command_obj) Exception.o Utility.o -lpthread
	
server: server.o $(trans_obj) $(reuse_sock_obj) $(server_obj) Semaphore.o $(traversal_command_obj) Exception.o Log.o
	g++ -o server server.o $(trans_obj) $(reuse_sock_obj) $(server_obj) Semaphore.o $(traversal_command_obj) Exception.o Log.o -lpthread
	
mytest.o: mytest.cpp \
	$(nat_traversal_inc_dir)/NatTraversalClient.h $(icf_nat_checker_client) \
	$(sock_inc_dir)/ClientSocket.h $(icf_client_socket)
	g++ -c mytest.cpp -std=c++11

client.o: main/client.cpp \
	$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
	$(nat_traversal_inc_dir)/NatTraversalClient.h $(icf_nat_traversal_client)
	g++ -c main/client.cpp -std=c++11
	
server.o: main/server.cpp \
	$(nat_traversal_inc_dir)/NatTraversalServer.h $(icf_nat_traversal_server)
	g++ -c main/server.cpp

########################################### object

TransmissionData.o: $(trans_inc_dir)/TransmissionData.h $(icf_trans_data) transmission/TransmissionData.cpp
	g++ -c transmission/TransmissionData.cpp

TransmissionProxy.o: transmission/TransmissionProxy.cpp $(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy) \
		$(json_inc_dir)/json.h $(icf_json) \
		$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
		$(trans_inc_dir)/TransmissionData.h $(icf_trans_data)
	g++ -c transmission/TransmissionProxy.cpp

json_reader.o: json/json_reader.cpp json/reader.h $(icf_reader_actually) json/value.h $(icf_value)
	g++ -c json/json_reader.cpp

json_writer.o: json/json_writer.cpp json/writer.h $(icf_writer_actually)
	g++ -c json/json_writer.cpp

json_value.o: json/json_value.cpp json/value.h $(icf_value) json/writer.h $(icf_writer_actually) 
	g++ -c json/json_value.cpp

ReuseClientSocket.o: socket/ReuseClientSocket.cpp \
		socket/ReuseClientSocket.h $(icf_reuse_client_socket)
	g++ -c socket/ReuseClientSocket.cpp

ReuseServerSocket.o: socket/ReuseServerSocket.cpp \
		socket/ReuseServerSocket.h $(icf_reuse_server_socket)
	g++ -c socket/ReuseServerSocket.cpp

ReuseSocketFactory.o: socket/ReuseSocketFactory.cpp \
		$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
		socket/ReuseServerSocket.h $(icf_reuse_server_socket) \
		socket/ReuseClientSocket.h $(icf_reuse_client_socket)
	g++ -c socket/ReuseSocketFactory.cpp
	
DefaultSocketFactory.o: $(sock_inc_dir)/DefaultSocketFactory.h $(icf_default_factory) \
			socket/DefaultSocketFactory.cpp \
			socket/DefaultServerSocket.h $(icf_default_server_socket) \
			socket/DefaultClientSocket.h $(icf_default_client_socket)
	g++ -c socket/DefaultSocketFactory.cpp
	
DefaultClientSocket.o: socket/DefaultClientSocket.h $(icf_default_client_socket) \
			socket/DefaultClientSocket.cpp
	g++ -c socket/DefaultClientSocket.cpp
	
DefaultServerSocket.o: socket/DefaultServerSocket.h $(icf_default_server_socket) \
			socket/DefaultServerSocket.cpp \
			socket/DefaultClientSocket.h $(icf_default_client_socket)
	g++ -c socket/DefaultServerSocket.cpp
	
Exception.o: include/Exception.h source/Exception.cpp
	g++ -c source/Exception.cpp
	
Semaphore.o: source/Semaphore.cpp include/Semaphore.h
	g++ -c source/Semaphore.cpp

Utility.o: source/Utility.cpp include/Utility.h \
			include/Exception.h $(icf_exception)
	g++ -c source/Utility.cpp

Log.o: source/Log.cpp include/Log.h $(icf_log)
	g++ -c source/Log.cpp -std=c++11 -I. -Iinclude
	
DefaultSocket.o: socket/DefaultSocket.h socket/DefaultSocket.cpp $(icf_default_socket)
	g++ -c socket/DefaultSocket.cpp
	
NatTraversalClient.o: nattraversal/NatTraversalClient.cpp \
					$(nat_traversal_inc_dir)/NatTraversalClient.h $(icf_nat_traversal_client) \
					$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
					$(trans_inc_dir)/TransmissionData.h $(icf_trans_data) \
					$(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy) \
					$(nat_checker_inc_dir)/NatCheckerClient.h $(icf_nat_checker_client) \
					socket/DefaultClientSocket.h $(icf_default_client_socket) \
					nattraversal/NatTraversalCommon.h $(icf_nat_traversal_common) \
					$(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command) \
					include/Utility.h $(icf_utility)
	g++ -c nattraversal/NatTraversalClient.cpp -std=c++11

NatTraversalServer.o: nattraversal/NatTraversalServer.cpp \
					$(nat_traversal_inc_dir)/NatTraversalServer.h $(icf_nat_traversal_server) \
					$(nat_checker_inc_dir)/NatCheckerServer.h $(icf_nat_checker_server) \
					$(trans_inc_dir)/TransmissionData.h $(icf_trans_data) \
					$(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy) \
					socket/DefaultClientSocket.h $(icf_default_client_socket) \
					socket/DefaultServerSocket.h $(icf_default_server_socket) \
					nattraversal/NatTraversalCommon.h $(icf_nat_traversal_common) \
					database/DefaultDataBase.h $(icf_default_database) \
					$(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command) \
					include/Log.h $(icf_log)
	g++ -c nattraversal/NatTraversalServer.cpp -std=c++11 -I. -Iinclude/

NatCheckerClient.o: natchecker/NatCheckerClient.cpp \
		$(nat_checker_inc_dir)/NatCheckerClient.h $(icf_nat_checker_client) \
		$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
		$(sock_inc_dir)/ServerSocket.h $(icf_server_socket) \
		$(trans_inc_dir)/TransmissionData.h $(icf_trans_data) \
		$(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy)
	g++ -c natchecker/NatCheckerClient.cpp

NatCheckerServer.o: natchecker/NatCheckerServer.cpp \
		$(nat_checker_inc_dir)/NatCheckerServer.h $(icf_nat_checker_server) \
		$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
		$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
		socket/DefaultClientSocket.h $(icf_default_client_socket) \
		socket/DefaultServerSocket.h $(icf_default_server_socket) \
		$(trans_inc_dir)/TransmissionData.h $(icf_trans_data) \
		$(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy) \
		$(database_inc_dir)/DataBase.h $(icf_databse) \
		include/Log.h $(icf_log)
	g++ -c natchecker/NatCheckerServer.cpp -std=c++11 -I. -Iinclude/
	
TraversalCommand.o: traversalcommand/TraversalCommand.cpp \
					$(traversal_command_inc_dir)/TraversalCommand.h $(icf_traversal_command) \
					traversalcommand/ConnectDirectlyCommand.h $(icf_connect_directly) \
					traversalcommand/ConnectAroundCommand.h $(icf_connect_around) \
					traversalcommand/ConnectRandomlyCommand.h $(icf_connect_randomly) \
					traversalcommand/ListenDirectlyCommand.h $(icf_listen_directly) \
					traversalcommand/ListenAndPunchCommand.h $(icf_listen_and_punch) \
					traversalcommand/ListenAndPunchRandomlyCommand.h $(icf_listen_and_punch_some)
	g++ -c traversalcommand/TraversalCommand.cpp
	
ConnectDirectlyCommand.o: traversalcommand/ConnectDirectlyCommand.cpp \
					traversalcommand/ConnectDirectlyCommand.h $(icf_connect_directly) \
					$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
					$(sock_inc_dir)/ClientSocket.h $(icf_client_socket)
	g++ -c traversalcommand/ConnectDirectlyCommand.cpp

ConnectAroundCommand.o: traversalcommand/ConnectAroundCommand.cpp \
					traversalcommand/ConnectAroundCommand.h $(icf_connect_around) \
					socket/ReuseClientSocket.h $(icf_reuse_client_socket) \
					include/SmartPointer.h $(icf_smart_pointer)
	g++ -c traversalcommand/ConnectAroundCommand.cpp -std=c++11

ConnectRandomlyCommand.o: traversalcommand/ConnectRandomlyCommand.cpp \
					traversalcommand/ConnectRandomlyCommand.h $(icf_connect_randomly) \
					socket/ReuseClientSocket.h $(icf_reuse_client_socket) \
					include/SmartPointer.h $(icf_smart_pointer)
	g++ -c traversalcommand/ConnectRandomlyCommand.cpp -std=c++11

ListenDirectlyCommand.o: traversalcommand/ListenDirectlyCommand.cpp \
					traversalcommand/ListenDirectlyCommand.h $(icf_listen_directly) \
					$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
					$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
					$(sock_inc_dir)/ServerSocket.h $(icf_server_socket) \
					socket/ReuseServerSocket.h $(icf_reuse_server_socket) \
					include/SmartPointer.h $(icf_smart_pointer)
	g++ -c traversalcommand/ListenDirectlyCommand.cpp -std=c++11
	
ListenAndPunchCommand.o: traversalcommand/ListenAndPunchCommand.cpp \
					traversalcommand/ListenAndPunchCommand.h $(icf_listen_and_punch) \
					$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
					$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
					$(sock_inc_dir)/ServerSocket.h $(icf_server_socket) \
					socket/ReuseServerSocket.h $(icf_reuse_server_socket) \
					include/SmartPointer.h $(icf_smart_pointer)
	g++ -c traversalcommand/ListenAndPunchCommand.cpp -std=c++11
	
ListenAndPunchRandomlyCommand.o: traversalcommand/ListenAndPunchCommand.cpp \
					traversalcommand/ListenAndPunchCommand.h $(icf_listen_and_punch_some) \
					socket/ReuseServerSocket.h $(icf_reuse_server_socket) \
					socket/ReuseClientSocket.h $(icf_reuse_client_socket) \
					include/SmartPointer.h $(icf_smart_pointer)
	g++ -c traversalcommand/ListenAndPunchRandomlyCommand.cpp -std=c++11

###########################################

uninstall_all:
	-rm client server

uninstall_client:
	rm client

uninstall_server:
	rm server

clean:
	rm -rf *.o

