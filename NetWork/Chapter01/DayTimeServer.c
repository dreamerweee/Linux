#include "../Util.h"
#include <time.h>

int main(int argc, char *argv[])
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		ErrSys("socket error");
	}

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(13);

	int ret = bind(listen_fd, (SA *)&serv_addr, sizeof(serv_addr));
	if (ret < 0) {
		ErrSys("bind error");
	}

	listen(listen_fd, 5);

	int conn_fd;
	char buff[MAXLINE];
	time_t ticks;
	while (1) {
		conn_fd = accept(listen_fd, (SA *)NULL, NULL);
		if (conn_fd < 0) {
			ErrSys("accept error");
		} else {
			ticks = time(NULL);
			snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
			write(conn_fd, buff, strlen(buff));
			close(conn_fd);
		}
	}
	exit(0);
}