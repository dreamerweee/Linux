#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MAX_LEN 1024

int main()
{
	int conn_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (conn_fd < 0) {
		printf("SimpleClient: socket error\n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(54320);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	connect(conn_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	const char *send_msg = "Hi, this is my msg.";
	ssize_t send_len = send(conn_fd, send_msg, strlen(send_msg), 0);
	printf("SimpleClient: send msg: %s, \nlen: %d\n", send_msg, send_len);

	char recv_mg[MAX_LEN];
	ssize_t recv_len = recv(conn_fd, recv_mg, MAX_LEN, 0);
	printf("SimpleClient: recv: %s, \nlen = %d\n", recv_mg, recv_len);

	exit(EXIT_SUCCESS);
}