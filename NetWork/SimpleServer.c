#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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

	while (1) {

	}

	exit(EXIT_SUCCESS);
}