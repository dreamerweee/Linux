#include "SocketPack.h"

/*
* 以connect为例，说明程序中如何使用SO_SNDTIMEO选项来定时
*/

// 超时连接函数
int TimeoutConnect(const char *ip, int port, int time)
{
	int conn_fd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	InetPton(AF_INET, ip, &serv_addr.sin_addr);

	struct timeval timeout;
	timeout.tv_sec = time;
	timeout.tv_usec = 0;
	socklen_t len = sizeof(timeout);
	int ret = setsockopt(conn_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
	if (ret < 0) {
		ErrSys("setsockopt error");
	}

	ret = connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret == -1) {
		// 超时对应的错误号是 EINPROGRESS，下面这个条件如果成立，我们就可以处理定时任务了
		if (errno == EINPROGRESS) {
			printf("connecting timeout, process timeout logic\n");
			return -1;
		}
		printf("connect error\n");
		return -1;
	}

	return conn_fd;
}

int main(int argc, char *argv[])
{
	if (argc <= 2) {
		printf("usage: %s <ip> <port>\n", basename(argv[0]));
		return 1;
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	int conn_fd = TimeoutConnect(ip, port, 10);
	if (conn_fd < 0) {
		return 1;
	}
	return 0;
}
