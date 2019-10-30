/*
* 多进程服务器：一个子进程处理一个客户连接。同时，我们将所有客户socket连接的读缓冲设计为一块共享内存
*/

#include "SocketPack.h"
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define USER_LIMIT 5
#define BUFFER_SIZE 1024
#define FD_LINIT 65535
#define PROCESS_LIMIT 65535

typedef struct ClientData {
	struct sockaddr_in cli_addr;
	int conn_fd;
	pid_t pid;    // 处理这个连接的子进程的PID
	int pipe_fd[2];   // 和父进程通信的管道
}ClientData;

static const char *sg_shm_name = "/my_shm";
int g_sig_pipe_fd[2];
int g_epoll_fd;
int g_listen_fd;
int g_shm_fd;
char *g_share_mem = 0;

// 客户连接数组，进程用客户连接的编号来索引这个数组，即可取得相关的客户连接数据
ClientData *g_users;

// 子进程和客户连接的映射关系表，用进程的PID来索引这个数组，即可取得该进程所处理的客户连接的编号
int *g_sub_process;

// 当前客户数量
int g_user_count = 0;

bool g_stop_child = false;

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
	send(g_sig_pipe_fd[1], (char *)&msg, 1, 0);
	errno = save_errno;
}

void AddSig(int signo, void (*handler)(int), bool restart = true)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if (restart) {
		sa.sa_flags |= SA_RESTART;
	}
	sigfillset(&sa.sa_mask);
	if (sigaction(signo, &sa, NULL) < 0) {
		ErrSys("sigaction error");
	}
}

void DelResource()
{
	Close(g_sig_pipe_fd[0]);
	Close(g_sig_pipe_fd[1]);
	Close(g_listen_fd);
	Close(g_epoll_fd);
	shm_unlink(sg_shm_name);
	delete [] g_users;
	delete [] g_sub_process;
}

// 停止一个子进程
void ChildTermHandler(int signo)
{
	g_stop_child = true;
}

/*
* 子进程运行的函数。
* 	参数idx指出该子进程处理的客户连接的编号，
* 	users是保存所有客户连接数据的数组，
* 	参数share_mem指出共享内存的起始地址
*/
int RunChild(int idx, ClientData *users, char *share_mem)
{
	struct epoll_event events[MAX_EVENT_NUMBER];
	// 子进程使用I/O复用技术来同时监听两个文件描述符：客户连接socket、与父进程通信的管道文件描述符
	int child_epfd = epoll_create(5);
	if (child_epfd < 0) {
		ErrSys("epoll_create error");
	}
	int conn_fd = users[idx].conn_fd;
	AddFD(child_epfd, conn_fd);

	int pipe_fd = users[idx].pipe_fd[1];
	AddFD(child_epfd, pipe_fd);

	int ret;

	// 子进程需要设置自己的信号处理函数
	AddSig(SIGTERM, ChildTermHandler, false);

	while (!g_stop_child) {
		int num = epoll_wait(child_epfd, events, MAX_EVENT_NUMBER, -1);
		if ((num < 0) && (errno != EINTR)) {
			printf("epoll wait failed\n");
			break;
		}

		for (int i = 0; i < num; ++i) {
			int sock_fd = events[i].data.fd;
			if ((sock_fd == conn_fd) && (events[i].events & EPOLLIN)) {
				memset(share_mem + idx * BUFFER_SIZE, '\0', BUFFER_SIZE);
				// 将客户数据读取到对应的读缓存中。
				// 该读缓存是共享内存的一段，它开始于idx * BUFFER_SIZE处，长度为BUFFER_SIZE字节
				ret = recv(conn_fd, share_mem + idx * BUFFER_SIZE, BUFFER_SIZE - 1, 0);
				if (ret < 0) {
					if (errno != EAGAIN) {
						g_stop_child = true;
					}
				} else if (ret == 0) {
					g_stop_child = true;
				} else {
					// 成功读取客户数据后就通知主进程（通过管道）来处理
					send(pipe_fd, (char *)&idx, sizeof(idx), 0)
				}
			} else if ((sock_fd == pipe_fd) && (events[i].events & EPOLLIN)) {
				// 主进程通知本进程（通过管道）将第client个客户的数据发送到本进程负责的客户端
				int client = 0;
				ret = recv(sock_fd, (char *)&client, sizeof(client), 0);
				if (ret < 0) {
					if (errno != EAGAIN) {
						g_stop_child = true;
					}
				} else if (ret == 0) {
					g_stop_child = true;
				} else {
					send(conn_fd, share_mem + client * BUFFER_SIZE, BUFFER_SIZE, 0);
				}
			} else {
				continue;
			}
		}
	}

	Close(conn_fd);
	Close(pipe_fd);
	Close(child_epfd);
	return 0;
}

int main(int argc, char *argv[])
{
	g_listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(g_listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(g_listen_fd, 5);

	g_user_count = 0;
	g_users = new ClientData[USER_LIMIT + 1];
	g_sub_process = new int[PROCESS_LIMIT]
	for (int i = 0; i < PROCESS_LIMIT; ++i) {
		g_sub_process[i] = -1;
	}

	struct epoll_event events[MAX_EVENT_NUMBER];
	g_epoll_fd = epoll_create(5);
	if (g_epoll_fd < 0) {
		ErrSys("epoll_create error");
	}
	AddFD(g_epoll_fd, g_listen_fd);

	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, g_sig_pipe_fd);
	if (ret < 0) {
		ErrSys("socketpair error");
	}
	SetNonblocking(g_sig_pipe_fd[1]);
	AddFD(g_epoll_fd, g_sig_pipe_fd[0]);

	AddSig(SIGCHLD, SigHandler);
	AddSig(SIGTERM, SigHandler);
	AddSig(SIGINT, SigHandler);
	AddSig(SIGPIPE, SigHandler);

	bool stop_server = false;
	bool terminate = false;

	// 创建共享内存，作为所有客户socket连接的读缓存
	g_shm_fd = shm_open(sg_shm_name, O_CREAT | O_RDWR, 0666);
	if (g_shm_fd < 0) {
		ErrSys("shm_open error");
	}
	ret = ftruncate(g_shm_fd, USER_LIMIT * BUFFER_SIZE);
	if (ret < 0) {
		ErrSys("ftruncate error");
	}

	g_share_mem = (char *)mmap(NULL, USER_LIMIT * BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_shm_fd, 0);
	if (g_share_mem == MAP_FAILED) {
		ErrSys("mmap error");
	}

	while (!stop_server) {
		int num = epoll_wait(g_epoll_fd, events, MAX_EVENT_NUMBER, -1);
		if ((num < 0) && (errno != EINTR)) {
			printf("epoll wait failed.\n");
			break;
		}

		for (int i = 0; i < num; ++i) {
			int sock_fd = events[i].data.fd;
			if (sock_fd == g_listen_fd) {
				struct sockaddr_in cli_addr;
				socklen_t cli_addr_len = sizeof(cli_addr);
				int conn_fd = accept(g_listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
				if (conn_fd < 0) {
					printf("accept error: %d\n", errno);
					continue;
				}
				if (g_user_count >= USER_LIMIT) {
					const char *msg = "too many users\n";
					printf("%s", msg);
					send(conn_fd, msg, strlen(msg), 0);
					Close(conn_fd);
					continue;
				}

				// 保存第g_user_count个客户连接的相关数据
				g_users[g_user_count].cli_addr = cli_addr;
				g_users[g_user_count].conn_fd = conn_fd;
				// 在主进程和子进程间建立管道，以传递必要的数据
				ret = socketpair(AF_UNIX, SOCK_STREAM, 0, g_users[g_user_count].pipe_fd);
				if (ret < 0) {
					printf("socketpair error\n");
					Close(conn_fd);
					continue;
				}
				pid_t pid = fork();
				if (pid < 0) {
					Close(conn_fd);
					continue
				} else if (pid == 0) {
					
				}
			}
		}
	}
}
