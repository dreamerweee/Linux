#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include "../SocketPack.h"
#include <sys/epoll.h>
#include <sys/wait.h>
#include <signal.h>

/*
* 描述一个子进程的类
*/
class Process {
public:
	Process() : m_pid(-1) { }

public:
	pid_t m_pid;   // 目标子进程的PID
	int m_pipefd[2]; // 父进程和子进程通信的管道
};

// 进程池
template <typename T>
class ProcessPool {
private:
	// 构造函数定义为私有的，因此我们只能通过后面的Create静态函数来创建ProcessPool实例
	ProcessPool(int listen_fd, int process_num);

public:
	// 单例模式，以保证程序最多创建一个ProcessPool实例，这是程序正确处理信号的必要条件
	static ProcessPool<T>* Create(int listen_fd, int process_num = 8)
	{
		if (!m_instance) {
			m_instance = new ProcessPool<T>(listen_fd, process_num);
		}
		return m_instance;
	}
	~ProcessPool()
	{
		delete [] m_sub_process;
	}

	// 启动进程池
	void Run();

private:
	void SetupSigPipe();
	void RunParent();
	void RunChild();

private:
	// 进程池允许的最大子进程数量
	static const int MAX_PROCESS_NUMBER = 16;
	// 每个子进程最多能处理的客户数量
	static const int MAX_USER_PER_PROCESS = 65535;
	// epoll最多能处理的事件数
	static const int MAX_EVENT_NUMBER1 = 10000;

	// 进程池中的进程总数
	int m_process_number;
	// 子进程在池中的序号，从0开始
	int m_idx;
	// 每个进程都有一个epoll内核事件表，用m_epollfd标识
	int m_epollfd;
	// 监听socket
	int m_listenfd;
	// 子进程通过m_stop来决定是否停止运行
	int m_stop;
	// 保存所有子进程的描述信息
	Process *m_sub_process;
	// 进程池静态实例
	static ProcessPool<T> *m_instance;
};

template<typename T>
ProcessPool<T> *ProcessPool<T>::m_instance = NULL;

// 用于处理信号的管道，以实现统一事件源
static int sig_pipefd[2];

static void AddFD(int epoll_fd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
		ErrSys("AddFD error");
	}
	SetNonblocking(fd);
}

void RemoveFD(int epoll_fd, int fd)
{
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
		ErrSys("RemoveFD error");
	}
	Close(fd);
}

static void SigHandler(int signo)
{
	int save_errno = errno;
	int msg = signo;
	send(sig_pipefd[1], (char *)&msg, 1, 0);
	errno = save_errno;
}

static void AddSig(int signo, void (handler)(int), bool restart = true)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if (restart) {
		sa.sa_flags |= SA_RESTART;
	}
	sigfillset(&sa.sa_mask);
	if (sigaction(signo, &sa, NULL) < 0) {
		ErrSys("AddSig error");
	}
}

/*
* 进程池构造函数，参数listen_fd是监听socket，它必须在创建进程池之前被创建，
* 否则子进程无法直接引用它。参数process_num指定进程池中子进程的数量
*/
template<typename T>
ProcessPool<T>::ProcessPool(int listen_fd, int process_num) :
	m_listenfd(listen_fd), m_process_number(process_num), m_idx(-1), m_stop(false)
{
	if (process_num <= 0 || process_num >= MAX_PROCESS_NUMBER) {
		ErrSys("ProcessPool init error");
	}

	m_sub_process = new Process[process_num];
	if (!m_sub_process) {
		ErrSys("ProcessPool new Process error");
	}

	// 创建process_num个子进程，并建立它们和父进程之间的管道
	for (int i = 0; i < process_num; ++i) {
		int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd);
		if (ret < 0) {
			ErrSys("ProcessPool socketpair error");
		}

		m_sub_process[i].m_pid = fork();
		if (m_sub_process[i].m_pid < 0) {
			ErrSys("ProcessPool fork error");
		}
		if (m_sub_process[i].m_pid > 0) {
			// 父进程
			Close(m_sub_process[i].m_pipefd[1]);
			continue;
		} else {
			// 子进程
			Close(m_sub_process[i].m_pipefd[0]);
			m_idx = i;
			break;
		}
	}
}

// 统一事件源
template<typename T>
void ProcessPool<T>::SetupSigPipe()
{
	// 创建epoll事件监听表和信号管道
	m_epollfd = epoll_create(5);
	if (m_epollfd < 0) {
		ErrSys("SetupSigPipe epoll_create error");
	}

	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sig_pipefd);
	if (ret < 0) {
		ErrSys("SetupSigPipe socketpair error");
	}

	SetNonblocking(sig_pipefd[1]);
	AddFD(m_epollfd, sig_pipefd[0]);

	// 设置信号处理函数
	AddSig(SIGCHLD, SigHandler);
	AddSig(SIGTERM, SigHandler);
	AddSig(SIGINT, SigHandler);
	AddSig(SIGPIPE, SIG_IGN);
}

// 父进程中m_idx值为-1，子进程中m_idx值大于等于0，我们据此判断接下来要运行的是父进程代码还是子进程代码
template<typename T>
void ProcessPool<T>::Run()
{
	if (m_idx != -1) {
		RunChild();
		return;
	}
	RunParent();
}

template<typename T>
void ProcessPool<T>::RunChild()
{
	SetupSigPipe();

	// 每个子进程都通过其在进程池中的序号值m_idx找到与父进程通信的管道
	int pipe_fd = m_sub_process[m_idx].m_pipefd[1];
	// 子进程需要监听管道文件描述符pipe_fd，因为父进程将通过它来通知子进程accept新连接
	AddFD(m_epollfd, pipe_fd);

	struct epoll_event events[MAX_EVENT_NUMBER1];
	T *users = new T[MAX_USER_PER_PROCESS];
	if (!users) {
		ErrSys("RunChild new T error");
	}
	int num = 0;
	int ret = -1;

	while (!m_stop) {
		num = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER1, -1);
		if (num < 0 && errno != EINTR) {
			printf("RunChild epoll wait failure\n");
			break;
		}

		for (int i = 0; i < num; ++i) {
			int sock_fd = events[i].data.fd;
			if ((sock_fd == pipe_fd) && (events[i].events & EPOLLIN)) {
				int client = 0;
				// 从父、子进程之间的管道读取数据，并将结果保存在变量client中
				// 如果读取成功，则表示有新客户连接到来
				ret = recv(sock_fd, (char *)&client, sizeof(client), 0);
				if (((ret < 0) && (errno != EAGAIN)) || ret == 0) {
					continue;
				} else {
					struct sockaddr_in cli_addr;
					socklen_t cli_addr_len = sizeof(cli_addr);
					int conn_fd = accept(m_listenfd, (struct sockaddr *)&cli_addr, &cli_addr_len);
					if (conn_fd < 0) {
						printf("RunChild accept errno is: %d\n", errno);
						continue;
					}
					AddFD(m_epollfd, conn_fd);
					// 模板类T必须实现Init方法，以初始化一个客户连接。
					// 我们直接使用conn_fd来索引逻辑处理对象(T类型的对象)，以提高程序效率
					users[conn_fd].Init(m_epollfd, conn_fd, cli_addr);
				}
			} else if ((sock_fd == sig_pipefd[0]) && (events[i].events & EPOLLIN)) {
				// 处理子进程接收到的信号
				char signals[1024];
				ret = recv(sock_fd, signals, sizeof(signals), 0);
				if (ret <= 0) {
					continue;
				} else {
					for (int j = 0; j < ret; ++j) {
						switch (signals[i]) {
							case SIGCHLD:
							{
								pid_t pid;
								int stat;
								while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
									continue;
								}
								break;
							}
							case SIGTERM:
							case SIGINT:
							{
								m_stop = true;
								break;
							}
							default:
								break;
						}
					}
				}
			} else if (events[i].events & EPOLLIN) {
				// 其他可读数据，必定是客户请求到来。调用逻辑处理对象的Process方法处理之
				users[sock_fd].Process();
			} else {
				continue;
			}
		}
	}

	delete [] users;
	users = NULL;
	Close(pipe_fd);
	Close(m_epollfd);
}


template<typename T>
void ProcessPool<T>::RunParent()
{
	SetupSigPipe();

	// 父进程监听m_listenfd
	AddFD(m_epollfd, m_listenfd);

	struct epoll_event events[MAX_EVENT_NUMBER1];
	int sub_process_counter = 0;
	int new_conn = 1;
	int num = 0;
	int ret = -1;

	while (!m_stop) {
		num = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER1, -1);
		if ((num < 0) && (errno != EINTR)) {
			printf("RunParent epoll_wait error\n");
			break;
		}

		for (int i = 0; i < num; ++i) {
			int sock_fd = events[i].data.fd;
			if (sock_fd == m_listenfd) {
				// 如果有新连接到来，就采用Round Robin方式将其分配给一个子进程处理
				int idx = sub_process_counter;
				do {
					if (m_sub_process[idx].m_pid != -1) {
						break;
					}
					idx = (idx + 1) % m_process_number;
				} while (idx != sub_process_counter);
				if (m_sub_process[idx].m_pid == -1) {
					m_stop = true;
					break;
				}
				sub_process_counter = (idx + 1) % m_process_number;
				send(m_sub_process[idx].m_pipefd[0], (char *)&new_conn, sizeof(new_conn), 0);
				printf("RunParent send request to child %d\n", idx);
			} else if ((sock_fd == sig_pipefd[0]) && (events[i].events & EPOLLIN)) {
				// 下面处理父进程接收到的信号
				char signals[1024];
				ret = recv(sock_fd, signals, sizeof(signals), 0);
				if (ret <= 0) {
					continue;
				} else {
					for (int j = 0; j < ret; ++j) {
						switch (signals[i]) {
							case SIGCHLD:
							{
								pid_t pid;
								int stat;
								while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
									for (int k = 0; k < m_process_number; ++k) {
										// 如果进程池中第i个子进程退出了，则主进程关闭相应的通信管道，并设置相应的m_pid为-1，以标记该子进程已经退出
										if (m_sub_process[k].m_pid == pid) {
											printf("RunParent child %d join\n", k);
											Close(m_sub_process[k].m_pipefd[0]);
											m_sub_process[k].m_pid = -1;
										}
									}
								}
								// 如果所有子进程都已经退出了，则父进程也退出
								m_stop = true;
								for (int k = 0; k < m_process_number; ++k) {
									if (m_sub_process[k].m_pid != -1) {
										m_stop = false;
										break;
									}
								}
								break;
							}
							case SIGTERM:
							case SIGINT:
							{
								// 如果父进程接收到终止信号，那么就杀死所有子进程，并等待它们全部结束。
								// 通知子进程结束更好的方法是向父、子进程之间的通信管道发送特殊数据
								printf("kill all the child now\n");
								for (int k = 0; k < m_process_number; ++k) {
									int pid = m_sub_process[k].m_pid;
									if (pid != -1) {
										kill(pid, SIGTERM);
									}
								}
								break;
							}
							default:
								break;
						}
					}
				}
			}
			else {
				continue;
			}
		}
	}

	Close(m_epollfd);
}

#endif  // PROCESS_POOL_H