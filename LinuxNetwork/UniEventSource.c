#include "SocketPack.h"
#include <sys/epoll.h>
#include <signal.h>

// 统一信号事件和IO事件

static int g_s_pipe_fd[2];

void AddFD(int epfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
		ErrSys("epoll_ctl error");
	}
	SetNonblocking(fd);
}

void SigHandler(int signo)
{
	int save_errno = errno;
	int msg = signo;
	send(g_s_pipe_fd[1], (char *)&msg, 1, 0);
	errno = save_errno;
}

void AddSig(int signo)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SigHandler;
	sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	if (sigaction(signo, &sa, NULL) < 0) {
		ErrSys("sigaction error");
	}
}

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

	struct epoll_event events[MAX_EVENT_NUMBER];
	int epfd = epoll_create(5);
	if (epfd < 0) {
		ErrSys("epoll_create error");
	}
	AddFD(epfd, listen_fd);

	// socketpair创建双向管道
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, g_s_pipe_fd);
	if (ret < 0) {
		ErrSys("socketpair error");
	}
	SetNonblocking(g_s_pipe_fd[1]);
	AddFD(epfd, g_s_pipe_fd[0]);

	AddSig(SIGHUP);
	AddSig(SIGCHLD);
	AddSig(SIGINT);
	AddSig(SIGTERM);

	int stop_flag = 0;
	while (!stop_flag) {
		int num = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
		if ((num < 0) && (errno != EINTR)) {
			ErrSys("epoll_wait error");
		}
		for (int i = 0; i < num; ++i) {
			int sock_fd = events[i].data.fd;
			if (sock_fd == listen_fd) {
				int conn_fd = Accept(listen_fd, NULL, NULL);
				AddFD(epfd, conn_fd);
			} else if ((sock_fd == g_s_pipe_fd[0]) && (events[i].events & EPOLLIN)) {
				int signo;
				char signals[1024];
				ret = recv(g_s_pipe_fd[0], signals, sizeof(signals), 0);
				if (ret <= 0) {
					continue;
				} else {
					// 每个信号值占1字节，所以按字节来逐个接收信号
					for (int j = 0; j < ret; ++j) {
						switch (signals[i]) {
							case SIGCHLD:
							case SIGHUP:
							{
								continue;
							}
							case SIGTERM:
							case SIGINT:
							{
								stop_flag = 1;
							}
						}
					}
				}
			} else {
				printf("Something else happened.\n");
			}
		}
	}
	printf("server close\n");
	Close(listen_fd);
	Close(g_s_pipe_fd[1]);
	Close(g_s_pipe_fd[0]);
	exit(0);
}
