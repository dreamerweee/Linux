#include "../SocketPack.h"

/*
* 考虑如下情况：如果监听队列中处于ESTABLISHED状态的连接对应的客户端出现网络异常（比如掉线）
* 或者提前退出，那么服务器对这个连接执行accept调用是否成功？
*/

int main(int argc, char *argv[])
{
	int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5678);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(listen_fd, 5);

	// 暂停20秒以等待客户端连接和相关操作（掉线或者退出）完成
	sleep(20);

	struct sockaddr_in cli_addr;
	socklen_t cli_len = sizeof(cli_addr);
	int conn_fd = Accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);

	char remote[INET_ADDRSTRLEN];
	InetNtop(AF_INET, &cli_addr.sin_addr, remote, INET_ADDRSTRLEN);
	printf("connected with ip[%s] and port[%d]\n", remote, ntohs(cli_addr.sin_port));
	Close(conn_fd);
	Close(listen_fd);

	exit(0);
}

/*
* 实验证明：accept只是从监听队列中取出连接，而不论连接处于何种状态(如上面的ESTABLISHED状态和CLOSE_WAIT状态)，更不关心任何网络状况的变化。
*/