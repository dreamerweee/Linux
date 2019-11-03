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

static void RemoveFD(int epoll_fd, int fd)
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

#endif  // PROCESS_POOL_H