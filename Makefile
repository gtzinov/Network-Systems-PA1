TARGETS = client server

all: $(TARGETS)

client: client_files/udp_client.c 
	gcc client_files/udp_client.c -o client_files/client 

server: server_files/udp_server.c 
	gcc server_files/udp_server.c -o server_files/server 

clean: 
	rm client_files/client server_files/server