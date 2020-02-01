#include "SocketPack.h"
#include <pthread.h>
#include <sys/epoll.h>

#define BUFFER_SIZE 4096

typedef struct epoll_event epoll_event;

typedef struct FdSet {
	int epoll_fd;
	int sock_fd;
} FdSet;

/*
* 将fd上的EPOLLIN和EPOLLET事件注册到epfd指示的epoll内核事件表中，
* 参数oneshot指定是否注册fd上的EPOLLONESHOT事件
*/
void AddFD(int epfd, int fd, int oneshot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if (oneshot) {
		event.events |= EPOLLONESHOT;
	}
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
		ErrSys("epoll_ctl error");
	}
	SetNonblocking(fd);
}

/*
* 重置fd上的事件
*/
void ResetOneshot(int epfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0) {
		ErrSys("epoll_ctl error");
	}
}

// 工作线程
void* WorkerThread(void *args)
{
	int epfd = ((FdSet *)args)->epoll_fd;
	int sock_fd = ((FdSet *)args)->sock_fd;
	printf("start new thread to receive data on fd: %d\n", sock_fd);
	char buff[BUFFER_SIZE + 1];
	memset(buff, '\0', BUFFER_SIZE + 1);
	while (1)  {
		int recv_len = recv(sock_fd, buff, BUFFER_SIZE, 0);
		if (recv_len < 0) {
			if (errno == EAGAIN) {
				ResetOneshot(epfd, sock_fd);
				printf("recv later\n");
				break;
			}
		} else if (recv_len == 0) {
			Close(sock_fd);
			printf("remote close the connection\n");
			break;
		} else {
			printf("get content: %s\n", buff);
			sleep(5); // 模拟数据处理过程
		}
	}
	printf("end thread receive data on fd: %d\n", sock_fd);
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

	epoll_event events[MAX_EVENT_NUMBER];
	int epfd = epoll_create(5);
	if (epfd < 0) {
		ErrSys("epoll_create error");
	}
	AddFD(epfd, listen_fd, 0);

	while (1) {
		int ret = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0) {
			ErrSys("epoll_wait error");
		}
		for (int i = 0; i < ret; ++i) {
			int sock_fd = events[i].data.fd;
			if (sock_fd == listen_fd) {
				int conn_fd = Accept(listen_fd, NULL, NULL);
				AddFD(epfd, conn_fd, 1);
			} else if (events[i].events & EPOLLIN) {
				pthread_t tid;
				FdSet fds;
				fds.epoll_fd = epfd;
				fds.sock_fd = sock_fd;
				pthread_create(&tid, NULL, WorkerThread, (void *)&fds);
			} else {
				printf("something else happened\n");
			}
		}
	}
	
	Close(listen_fd);
	exit(0);
}
