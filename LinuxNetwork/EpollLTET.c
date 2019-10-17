#include "SocketPack.h"
#include <sys/epoll.h>

#define BUFFER_SIZE 10
#define MAX_EVENT_NUMBER 1024
#define TRUE 1
#define FALSE 0

/*
* 将fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中
* 参数et_flag指定是否启用ET模式
*/
void AddFD(int epfd, int fd, int et_flag)
{
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (et_flag) {
		event.events |= EPOLLET;
	}
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
	if (ret < 0) {
		ErrSys("epoll_ctl error");
	}
	SetNonblocking(fd);
}


/*
* LT模式的工作流程
*/
void LTMode(struct epoll_event *events, int number, int epfd, int listen_fd)
{
	char buff[BUFFER_SIZE];
	for (int i = 0; i < number; ++i) {
		int sock_fd = events[i].data.fd;
		if (sock_fd == listen_fd) {
			struct sockaddr_in cli_addr;
			socklen_t cli_addr_len = sizeof(cli_addr);
			int conn_fd = Accept(sock_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
			AddFD(epfd, conn_fd, FALSE);
		} else if (events[i].events & EPOLLIN) {
			printf("LT mode event trigger once\n");
			memset(buff, '\0', BUFFER_SIZE);
			int recv_len = recv(sock_fd, buff, BUFFER_SIZE - 1, 0);
			if (recv_len <= 0) {
				printf("LT mode socket fd close\n");
				Close(sock_fd);
				continue;
			}
			printf("LT get %d bytes of message: %s\n", recv_len, buff);
		} else {
			printf("other thing happened\n");
		}
	}
}

/*
* ET模式的工作流程
*/
void ETMode(struct epoll_event *events, int number, int epfd, int listen_fd)
{
	char buff[BUFFER_SIZE];
	for (int i = 0; i < number; ++i) {
		int sock_fd = events[i].data.fd;
		if (sock_fd == listen_fd) {
			struct sockaddr_in cli_addr;
			socklen_t cli_addr_len = sizeof(cli_addr);
			int conn_fd = Accept(sock_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
			AddFD(epfd, conn_fd, TRUE);
		} else if (events[i].events & EPOLLIN) {
			printf("ET mode event trigger once\n");
			while (1) {
				memset(buff, '\0', BUFFER_SIZE);
				int recv_len = recv(sock_fd, buff, BUFFER_SIZE - 1, 0);
				if (recv_len < 0) {
					// 对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕，此后，epoll就能再次出发sockfd上的EPOLLIN事件，以驱动下一次读操作
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
						printf("recv later\n");
						break;
					}
					Close(sock_fd);
					break;
				} else if (recv_len == 0) { // 对端关闭
					Close(sock_fd);
				} else {
					printf("ET get %d bytes of message: %s\n", recv_len, buff);
				}
			}
		} else {
			printf("Something else happened\n");
		}
	}
}

int main(int argc, char *argv[])
{
	int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
	int on_reuseaddr = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on_reuseaddr, sizeof(on_reuseaddr));

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(listen_fd, 5);

	struct epoll_event events[MAX_EVENT_NUMBER];
	int epfd = epoll_create(5);
	if (epfd < 0) {
		ErrSys("epoll_create error");
	}
	AddFD(epfd, listen_fd, TRUE);

	while (1) {
		int ret = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0) {
			printf("epoll wait failed.\n");
			break;
		}
		LTMode(events, ret, epfd, listen_fd);
		// ETMode(events, ret, epfd, listen_fd);  // 使用ET模式
	}

	Close(listen_fd);
	exit(0);
}
