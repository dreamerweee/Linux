#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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

	exit(EXIT_SUCCESS);
}