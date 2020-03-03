all: echo_server echo_client udp_echo_serv udp_echo_client local_server local_client

echo_server : echo_server2.c
	gcc -std=gnu99 -o ./build/echo_server echo_server2.c

echo_client : echo_client2.c
	gcc -std=gnu99 -o ./build/echo_client echo_client2.c

udp_echo_serv : udp_echo_serv.c
	gcc -std=gnu99 -o ./build/udp_echo_serv udp_echo_serv.c

udp_echo_client : udp_echo_client.c
	gcc -std=gnu99 -o ./build/udp_echo_client udp_echo_client.c

local_server : local_server.c
	gcc -std=gnu99 -o ./build/local_server local_server.c

local_client : local_client.c
	gcc -std=gnu99 -o ./build/local_client local_client.c

clean:
	rm ./build/echo_server ./build/echo_client ./build/udp_echo_serv ./build/udp_echo_client