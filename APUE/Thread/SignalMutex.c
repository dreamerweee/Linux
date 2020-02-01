/*Linux 环境编程：从应用到内核中的一个死锁例子*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static const char *const caller[2] = {"mutex_thread", "signal handler"};
static pthread_t mutex_tid;
static pthread_t sleep_tid;
static volatile int signal_handler_exit = 0;

static void HoldMutex(int c)
{
	printf("enter HoldMutex [caller %s]\n", caller[c]);
	pthread_mutex_lock(&mutex);
	// 这里的循环是为了保证锁不会在信号处理函数退出前被释放掉
	while (!signal_handler_exit && c != 1) {
		sleep(5);
	}
	pthread_mutex_unlock(&mutex);
	printf("leave HoldMutex [caller %s]\n", caller[c]);
}

static void* MutexThread(void *args)
{
	HoldMutex(0);
	return NULL;
}

static void* SleepThread(void *args)
{
	sleep(10);
	return NULL;
}

static void SignalHandler(int signum)
{
	HoldMutex(1);
	signal_handler_exit = 1;
}

int main()
{
	signal(SIGUSR1, SignalHandler);
	pthread_create(&mutex_tid, NULL, MutexThread, NULL);
	pthread_create(&sleep_tid, NULL, SleepThread, NULL);
	// sleep(10);
	pthread_kill(sleep_tid, SIGUSR1);
	pthread_join(mutex_tid, NULL);
	pthread_join(sleep_tid, NULL);
	return 0;
}

/*
	enter HoldMutex [caller mutex_thread]
	enter HoldMutex [caller signal handler]
	应该是这种情况下发生死锁
*/