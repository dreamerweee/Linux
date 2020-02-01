#include "../Util.h"

int main(int argc, char *argv[])
{
	if (argc != 2 ) {
		ErrQuit("usage: %s IPaddress", argv[0]);
	}

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		ErrSys("socket error");
	}

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(13);  // daytime server port 13
	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
		ErrSys("inet_pton error for %s", argv[1]);
	}
	if (connect(sock_fd, (SA *)&serv_addr, sizeof(serv_addr)) < 0) {
		ErrSys("connect error");
	}

	char recv_line[MAXLINE + 1];
	ssize_t read_len;
	while ((read_len = read(sock_fd, recv_line, MAXLINE)) > 0) {
		recv_line[read_len] = 0;  //末尾结束null字符
		if (fputs(recv_line, stdout) == EOF) {
			ErrSys("fputs error");
		}
	}
	if (read_len < 0) {
		ErrSys("read error");
	}
	exit(0);
}