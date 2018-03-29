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
	
##########################################
	 
sock_inc_dir = include/socket
json_inc_dir = include/json
trans_inc_dir = include/transmission
nat_checker_inc_dir = include/natchecker
database_inc_dir = include/database

########################################## base

icf_exception = 
icf_nat_type = 
icf_object = include/Exception.h $(icf_exception)
ifc_smart_pointer = include/Exception.h $(icf_exception)

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
icf_trans_proxy = include/Object.h $(icf_object) \
		$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
		$(trans_inc_dir)/TransmissionData.h $(icf_trans_data)
		
########################################### nat_checker

icf_nat_checker_common = include/NatType.h $(icf_nat_type)
icf_nat_checker_client = include/Object.h $(icf_object) \
						$(nat_checker_inc_dir)/NatCheckerCommon.h $(icf_nat_checker_common) \
						$(sock_inc_dir)/ClientSocket.h $(icf_client_socket)
icf_nat_checker_server = include/Object.h $(icf_object) \
						$(nat_checker_inc_dir)/NatCheckerCommon.h $(icf_nat_checker_common) \
						$(sock_inc_dir)/ServerSocket.h $(icf_server_socket)
						
########################################### database

icf_database = include/NatType.h $(icf_nat_type) \
				include/Object.h $(icf_object)
icf_database_factory = include/Object.h $(icf_object)
icf_default_database = $(database_inc_dir)/DataBase.h $(icf_databse)

########################################### target 

all: client server

test: mytest.o $(trans_obj) $(reuse_sock_obj) NatCheckerServer.o Exception.o
	g++ -o mytest mytest.o $(trans_obj) $(reuse_sock_obj) NatCheckerServer.o Exception.o -lpthread

client: client.o $(trans_obj) $(reuse_sock_obj) Exception.o
	g++ -o client client.o $(trans_obj) $(reuse_sock_obj) Exception.o
	
server: server.o $(trans_obj) $(reuse_sock_obj) Exception.o
	g++ -o server server.o $(trans_obj) $(reuse_sock_obj) Exception.o

########################################### object

TransmissionData.o: $(trans_inc_dir)/TransmissionData.h $(icf_trans_data) transmission/TransmissionData.cpp
	g++ -c transmission/TransmissionData.cpp

TransmissionProxy.o: transmission/TransmissionProxy.cpp $(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy) \
		$(json_inc_dir)/json.h $(icf_json)
	g++ -c transmission/TransmissionProxy.cpp

json_reader.o: json/json_reader.cpp json/reader.h $(icf_reader_actually) json/value.h $(icf_value)
	g++ -c json/json_reader.cpp

json_writer.o: json/json_writer.cpp json/writer.h $(icf_writer_actually)
	g++ -c json/json_writer.cpp

json_value.o: json/json_value.cpp json/value.h $(icf_value) json/writer.h $(icf_writer_actually) 
	g++ -c json/json_value.cpp
	
mytest.o: mytest.cpp \
	$(nat_checker_inc_dir)/NatCheckerClient.h $(icf_nat_checker_client)
	g++ -c mytest.cpp

client.o: main/client.cpp \
	$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
	$(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
	$(trans_inc_dir)/TransmissionData.h $(icf_trans_data) \
	$(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy)
	g++ -c main/client.cpp
	
server.o: main/server.cpp \
	$(sock_inc_dir)/ReuseSocketFactory.h $(icf_reuse_factory) \
	$(sock_inc_dir)/ServerSocket.h $(icf_server_socket) \
	$(trans_inc_dir)/TransmissionData.h $(icf_trans_data) \
	$(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy)
	g++ -c main/server.cpp

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
	
Exception.o:include/Exception.h source/Exception.cpp
	g++ -c source/Exception.cpp
	
DefaultSocket.o: socket/DefaultSocket.h socket/DefaultSocket.cpp $(icf_default_socket)
	g++ -c socket/DefaultSocket.cpp

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
		$(trans_inc_dir)/TransmissionData.h $(icf_trans_data) \
		$(trans_inc_dir)/TransmissionProxy.h $(icf_trans_proxy) \
		$(database_inc_dir)/DataBase.h $(icf_database)
		
	g++ -c natchecker/NatCheckerServer.cpp -std=c++11
	
DefaultDataBase.o: database/DefaultDataBase.cpp \
		database/DefaultDataBase.h $(icf_default_database)
	g++ -c database/DefaultDataBase.cpp
		
DataBaseFactory.o: database/DataBaseFactory.cpp \
		$(database_inc_dir)/DataBaseFactory.h $(icf_database_factory)
	g++ -c database/DataBaseFactory.cpp

###########################################

uninstall_all:
	-rm client server

uninstall_client:
	rm client

uninstall_server:
	rm server

clean:
	rm -rf *.o

