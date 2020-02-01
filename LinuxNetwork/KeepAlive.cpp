#include "SocketPack.h"
#include "ListTimer.h"
#include <signal.h>
#include <iostream>
#include <sys/epoll.h>

using std::cout;
using std::endl;

/*
* 应用层实现处理非活动连接的功能
*/

#define FD_LIMIT 65535
#define TIME_SLOT 5

static int gs_pipefd[2];
static SortTimerList gs_timer_list;
static int gs_epoll_fd = 0;

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
	send(gs_pipefd[1], (char *)&msg, 1, 0);
	errno = save_errno;
}

void AddSig(int signo)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SigHandler;
	sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	if (sigaction(signo, &sa, NULL) == -1) {
		ErrSys("sigaction error");
	}
}

void TimerHandler()
{
	// 定时器处理任务
	gs_timer_list.Tick();
	// 因为一次alarm调用只会引起一次SIGALRM信号，所以我们需要重新定时，以不断触发SIGALRM信号
	alarm(TIME_SLOT);
}

// 回调函数，删除非活动连接socket上的注册事件，并关闭
void CallBackFunc(ClientData *user_data)
{
	epoll_ctl(gs_epoll_fd, EPOLL_CTL_DEL, user_data->sock_fd, 0);
	Close(user_data->sock_fd);
	cout << "close fd " << user_data->sock_fd << endl;
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
	AddFD(epfd, listen_fd);
	gs_epoll_fd = epfd;

	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, gs_pipefd);
	if (ret == -1) {
		ErrSys("socketpair error");
	}
	SetNonblocking(gs_pipefd[1]);
	AddFD(epfd, gs_pipefd[0]);

	// 设置信号处理函数
	AddSig(SIGALRM);
	AddSig(SIGTERM);

	bool stop_server = false;

	ClientData *users = new ClientData[FD_LIMIT];
	bool timeout = false;
	alarm(TIME_SLOT);  // 定时

	while (!stop_server) {
		int num = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
		if ((num < 0) && (errno != EINTR)) {
			cout << "epoll failed" << endl;
			break;
		}
		for (int i = 0; i < num; ++i) {
			int sock_fd = events[i].data.fd;
			if (sock_fd == listen_fd) {
				struct sockaddr_in cli_addr;
				socklen_t cli_addr_len = sizeof(cli_addr);
				int conn_fd = Accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
				AddFD(epfd, conn_fd);
				// 加入 gs_timer_list
				users[conn_fd].cli_addr = cli_addr;
				users[conn_fd].sock_fd = conn_fd;

				UtilTimer *timer = new UtilTimer;
				timer->m_user_data = &users[conn_fd];
				timer->cb_func = CallBackFunc;
				time_t cur = time(NULL);
				timer->m_expire = cur + 3 * TIME_SLOT;
				users[conn_fd].timer = timer;
				gs_timer_list.AddTimer(timer);
			} else if ((sock_fd == gs_pipefd[0]) && events[i].events & EPOLLIN) {
				int signo;
				char signals[1024];
				ret = recv(gs_pipefd[0], signals, sizeof(signals), 0);
				if (ret == -1) {
					// handle the error
					continue;
				} else if (ret == 0) {
					continue;
				} else {
					for (int i = 0; i < ret; ++i) {
						switch (signals[i]) {
							// 用timeout变量标记有定时任务需要处理，但不立即处理定时任务。
							// 这是因为定时任务的优先级不是很高，我们有限处理其他更重要的任务
							case SIGALRM:
							{
								timeout = true;
								break;
							}
							case SIGTERM:
							{
								stop_server = true;
								break;
							}
						}
					}
				}
			} else if (events[i].events & EPOLLIN) {
				memset(users[sock_fd].buff, '\0', BUFFER_SIZE);
				ret = recv(sock_fd, users[sock_fd].buff, BUFFER_SIZE - 1, 0);
				cout << "get " << ret << " bytes of client data " 
					<< users[sock_fd].buff << " from " << sock_fd << endl;

				UtilTimer *timer = users[sock_fd].timer;
				if (ret < 0) {
					// 如果发送读错误，则关闭连接，并移除对应的定时器
					if (errno != EAGAIN) {
						CallBackFunc(&users[sock_fd]);
						if (timer) {
							gs_timer_list.DelTimer(timer);
						}
					}
				} else if (ret == 0) {
					// 如果对方已经关闭连接，则我们也关闭连接，并移除对应的定时器
					CallBackFunc(&users[sock_fd]);
					if (timer) {
						gs_timer_list.DelTimer(timer);
					}
				} else {
					// 如果某个客户连接上有数据可读，则我们需要调整该连接对应的定时器，以延迟该连接被关闭的时间
					if (timer) {
						time_t cur = time(NULL);
						timer->m_expire = cur + 3 * TIME_SLOT;
						cout << "Adjust timer once" << endl;
						gs_timer_list.AdjustTimer(timer);
					}
				}
			}
		}
		if (timeout) {
			TimerHandler();
			timeout = false;
		}
	}

	Close(listen_fd);
	Close(gs_pipefd[1]);
	Close(gs_pipefd[0]);
	delete [] users;
	exit(0);
}
