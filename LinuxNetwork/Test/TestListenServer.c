#include "../SocketPack.h"

/*
* 测试listen函数的backlog参数
*/

int main()
{
	int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5678);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(listen_fd, 3);

	while (1) {
		int rw_fd = accept(listen_fd, NULL, NULL);
		if (rw_fd < 0) {
			ErrSys("accept error");
		}
		printf("accept fd[%d]\n", rw_fd);
		fflush(stdout);
		sleep(100000);
	}

	exit(0);
}