#include "../SocketPack.h"

int main(int argc, char *argv[])
{
	if (argc <= 2) {
		ErrExit("usage: %s <ip> <port>", argv[0]);
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	int conn_fd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	InetPton(AF_INET, ip, &serv_addr.sin_addr);

	Connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	const char *oob_data =  "abc";
	const char *normal_data = "123";
	send(conn_fd, normal_data, strlen(normal_data), 0);
	send(conn_fd, oob_data, strlen(oob_data), MSG_OOB);
	send(conn_fd, normal_data, strlen(normal_data), 0);

	Close(conn_fd);
	exit(0);
}