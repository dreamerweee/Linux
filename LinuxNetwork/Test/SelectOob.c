#include "../SocketPack.h"
#include <sys/select.h>

int main(int argc, char *argv[])
{
	int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(listen_fd, 5);

	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);

	int conn_fd = Accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
	
	char buff[MAX_LINE_BUFF];
	fd_set read_fds;
	fd_set except_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&except_fds);

	int max_fd_p1 = conn_fd + 1;

	while (1) {
		memset(buff, '\0', sizeof(buff));

		FD_SET(conn_fd, &read_fds);
		FD_SET(conn_fd, &except_fds);
		int ret = select(max_fd_p1, &read_fds, NULL, &except_fds, NULL);
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}
			printf("select error: %s\n", strerror(errno));
			break;
		}
		if (FD_ISSET(conn_fd, &except_fds)) {
			ret = recv(conn_fd, buff, MAX_LINE_BUFF - 1, MSG_OOB);
			if (ret <= 0) {
				printf("recv oob error\n");
				break;
			}
			char remote[INET_ADDRSTRLEN];
			InetNtop(AF_INET, &cli_addr.sin_addr, remote, INET_ADDRSTRLEN);
			printf("recv bytes[%d] oob data[%s] from ip[%s] port[%d]\n", ret, buff,
				remote, ntohs(cli_addr.sin_port));
		} else if (FD_ISSET(conn_fd, &read_fds)) {
			ret = recv(conn_fd, buff, MAX_LINE_BUFF - 1, 0);
			if (ret <= 0) {
				printf("recv error\n");
				break;
			}
			char remote[INET_ADDRSTRLEN];
			InetNtop(AF_INET, &cli_addr.sin_addr, remote, INET_ADDRSTRLEN);
			printf("recv bytes[%d] data[%s] from ip[%s] port[%d]\n", ret, buff,
				remote, ntohs(cli_addr.sin_port));
		}
	}

	Close(conn_fd);
	Close(listen_fd);
	exit(0);
}