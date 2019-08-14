#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define MAX_LEN 1024

int CreateSocket(uint16_t port)
{
	int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		printf("CreateSocket: socket error[%d]\n", listen_fd);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret < 0) {
		printf("CreateSocket: bind error[%d]\n", ret);
		exit(EXIT_FAILURE);
	}

	ret = listen(listen_fd, 5);
	if (ret < 0) {
		printf("CreateSocket: listen error[%d]\n", ret);
		exit(EXIT_FAILURE);
	}

	return listen_fd;
}

int main()
{
	uint16_t port = 54320;
	int listen_fd = CreateSocket(port);

	struct sockaddr_in cli_addr;
	socklen_t cli_len = sizeof(cli_addr);
	bzero(&cli_addr, cli_len);

	const char *send_msg = "Hello, I had received your msg.";

	printf("SimpleServer: begin accept:\n");
	while (1) {
		int conn_sock = accept(listen_fd, (struct sockaddr*)&cli_addr, &cli_len);
		if (conn_sock < 0) {
			printf("SimpleServer accept falied error[%d]\n", errno);
		} else {
			char cli_ip[16];
			inet_ntop(AF_INET, &cli_addr.sin_addr, cli_ip, 16);
			printf("SimpleServer: accept address[%s] port[%d]\n", cli_ip, ntohs(cli_addr.sin_port));
			
			char msg[MAX_LEN];
			ssize_t recv_len = recv(conn_sock, msg, MAX_LEN, 0);
			printf("Recv msg: %s, \nlen: %d\n", msg, recv_len);

			send(conn_sock, send_msg, strlen(send_msg), 0);
		}
	}

	exit(EXIT_SUCCESS);
}