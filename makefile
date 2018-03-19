Object = DefaultClientSocket.o \
	 DefaultServerSocket.o \
	 DefaultSocketFactory.o \
	 DefaultSocket.o \
	 Exception.o
	 
sock_inc_dir = include/socket
json_inc_dir = include/json
trans_inc_dir = include/transmission

########################################## socket

icf_exception = 
icf_object = include/Exception.h $(icf_exception)

icf_socket = include/Object.h $(icf_object)
icf_client_socket = $(sock_inc_dir)/Socket.h $(icf_socket)
icf_server_socket = $(sock_inc_dir)/Socket.h $(icf_socket)
icf_default_factory = include/Object.h $(icf_object)
icf_reuse_factory = include/Object.h $(icf_object)

icf_default_socket = include/Object.h $(icf_object)
icf_default_client_socket = socket/DefaultSocket.h $(icf_default_socket)  $(sock_inc_dir)/ClientSocket.h $(icf_client_socket)
icf_default_server_socket = socket/DefaultSocket.h $(icf_default_socket)  $(sock_inc_dir)/ServerSocket.h $(icf_server_socket)

icf_reuse_client_socket = socket/DefaultClientSocket.h $(icf_default_client_socket)
icf_reuse_server_socket = socket/DefaultServerSocket.h $(icf_default_server_socket)

########################################### json

icf_config = 
icf_forwards = json/config.h $(icf_config)
icf_features = json/forwards.h $(icf_forwards)
icf_autolink = json/config.h $(icf_config)

icf_value = json/forwards.h $(icf_forwards)
icf_writer_actually = json/value.h $(icf_value)
icf_reader_actually = json/features.h $(icf_features) json/value.h $(icf_value)
icf_json_actually = json/autolink.h $(icf_autolink) json/reader.h $(icf_reader_actually) json/writer.h $(icf_writer_actually)

icf_json = json/json.h $(icf_json_actually)
icf_reader = json/reader.h $(icf_reader_actually)
icf_writer = json/writer.h $(icf_writer_actually)

########################################### transmission

icf_trans_data = include/Object.h $(icf_object)
icf_trans_proxy = include/Object.h $(icf_object) $(sock_inc_dir)/ClientSocket.h $(icf_client_socket) \
			$(trans_inc_dir)/TransmissionData.h $(icf_trans_data)

########################################### target 

test: mytest.o TransmissionData.o TransmissionProxy.o json_reader.o json_writer.o json_value.o Exception.o
	g++ -o mytest mytest.o TransmissionData.o TransmissionProxy.o json_reader.o json_writer.o json_value.o Exception.o

client: client.o $(Object)
	g++ -o client client.o $(Object)
	
server: server.o $(Object)
	g++ -o server server.o $(Object)

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
	
mytest.o: mytest.cpp $(trans_inc_dir)/TransmissionData.h $(icf_trans_data) $(json_inc_dir)/json.h $(icf_json)
	g++ -c mytest.cpp

client.o: client.cpp \
	$(sock_inc_dir)/DefaultSocketFactory.h \
	$(sock_inc_dir)/ClientSocket.h \
	$(sock_inc_dir)/Socket.h \
	include/Object.h \
	include/Exception.h
	g++ -c client.cpp
	
server.o: server.cpp \
	$(sock_inc_dir)/DefaultSocketFactory.h \
	$(sock_inc_dir)/ServerSocket.h \
	$(sock_inc_dir)/Socket.h \
	include/Object.h \
	include/Exception.h
	g++ -c server.cpp

###########################################
	
DefaultSocketFactory.o: $(sock_inc_dir)/DefaultSocketFactory.h \
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

