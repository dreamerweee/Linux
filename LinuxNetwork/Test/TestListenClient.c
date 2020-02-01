#include "../SocketPack.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		ErrExit("usage: %s <build count>", argv[0]);
	}

	int client_cnt = atoi(argv[1]);
	printf("client_cnt = %d\n", client_cnt);
	if (client_cnt < 1) {
		ErrExit("need build client count > 0");
	}

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5678);
	InetPton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	for (int i = 0; i < client_cnt; ++i) {
		printf("begin connect...\n");
		int client_fd = Socket(AF_INET, SOCK_STREAM, 0);
		connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	}

	while (1);

	exit(0);
}