#include "../SocketPack.h"

int main()
{
	int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5678);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(listen_fd, 5);

	int conn_fd = Accept(listen_fd, NULL, NULL);

	char buff[MAX_LINE_BUFF];
	memset(buff, '\0', MAX_LINE_BUFF);
	int recv_len = recv(conn_fd, buff, MAX_LINE_BUFF - 1, 0);
	printf("get %d bytes of normal data '%s'\n", recv_len, buff);

	memset(buff, '\0', MAX_LINE_BUFF);
	recv_len = recv(conn_fd, buff, MAX_LINE_BUFF - 1, MSG_OOB);
	printf("get %d bytes of oob data[%s]\n", recv_len, buff);

	memset(buff, '\0', MAX_LINE_BUFF);
	recv_len = recv(conn_fd, buff, MAX_LINE_BUFF - 1, 0);
	printf("get %d bytes of normal data[%s]\n", recv_len, buff);

	Close(conn_fd);

	Close(listen_fd);
	exit(0);
}