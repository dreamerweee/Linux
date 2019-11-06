#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <list>
#include <iostream>
#include <cstdio>
#include <pthread.h>
#include "Locker.h"

using namespace std;

template <typename T>
class ThreadPool {
public:
	// max_requests是请求队列中最多允许的、等待处理的请求的数量
	ThreadPool(int thread_number = 8, int max_requests = 10000);
	~ThreadPool();

	// 往请求队列中添加任务
	bool Append(T *request);

private:	
	// 工作线程运行的函数，它不断从工作队列中取出任务并执行之
	static void* Worker(void *arg);
	void Run();

private:
	int m_thread_number;   // 线程池中的线程数
	int m_max_requests;    // 请求队列中允许的最大请求数
	pthread_t *m_threads;  // 描述线程池的数组，其大小为m_thread_number
	list<T*> m_work_queue; // 请求队列
	Locker m_queue_locker; // 保护请求队列的互斥锁
	Sem m_queue_stat;      // 是否有任务需要处理
	bool m_stop;           // 是否结束线程
};

template <typename T>
ThreadPool<T>::ThreadPool(int thread_number, int max_requests) :
    m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(NULL)
{
	if ((thread_number <= 0) || (max_requests <= 0)) {
		cout << "ThreadPool init error" << endl;
		exit(-1);
	}
	m_threads = new pthread_t[m_thread_number];
	if (!m_threads) {
		cout << "ThreadPool new pthreads error" << endl;
		exit(-1);
	}

	// 创建thread_number个线程，并将它们都设置为脱离线程
	for (int i = 0; i < thread_number; ++i) {
		cout << "create the " << i << "th thread" << endl;
		if (pthread_create(m_threads + i, NULL, Worker, this) != 0) {
			delete [] m_threads;
			cout << "ThreadPool pthread_create error" << endl;
			exit(-1);
		}
		if (pthread_detach(m_threads[i]) != 0) {
			delete [] m_threads;
			cout << "ThreadPool pthread_detach error" << endl;
			exit(-1);
		}
	}
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
	delete [] m_threads;
	m_stop = true;
}

template <typename T>
bool ThreadPool<T>::Append(T *request)
{
	m_queue_locker.Lock();
	if (m_work_queue.size() >= m_max_requests) {
		m_queue_locker.Unlock();
		return false;
	}
	m_work_queue.push_back(request);
	m_queue_locker.Unlock();
	m_queue_stat.Post();
	return true;
}

template <typename T>
void* ThreadPool<T>::Worker(void *arg)
{
	ThreadPool *pool = (ThreadPool *)arg;
	pool->Run();
	return pool;
}

template <typename T>
void ThreadPool<T>::Run()
{
	while (!m_stop) {
		m_queue_stat.wait();
		m_queue_locker.Lock();
		if (m_work_queue.empty()) {
			m_queue_locker.Unlock();
			continue;
		}
		T *request = m_work_queue.front();
		m_work_queue.pop_front();
		m_queue_locker.Unlock();
		if (!request) {
			continue;
		}
		request->Process();
	}
}

#endif  //THREAD_POOL_H