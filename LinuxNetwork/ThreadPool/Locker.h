#ifndef LOCKER_H
#define LOCKER_H

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>

class Sem {
public:
	Sem()
	{
		if (sem_init(&m_sem, 0, 0) ! = 0) {
			std::cout << "Sem init error" << std::endl;
			exit(-1);
		}
	}
	~Sem()
	{
		sem_destroy(&m_sem);
	}
	// 等待信号量
	bool Wait()
	{
		return sem_wait(&m_sem) == 0;
	}
	// 增加信号量
	bool Post()
	{
		return sem_post(&m_sem) == 0;
	}

private:
	sem_t m_sem;
};

class Locker {
public:
	Locker()
	{
		if (pthread_mutex_init(&m_mutex, NULL) != 0) {
			std::cout << "Locker init error" << std::endl;
			exit(-1);
		}
	}
	~Locker()
	{
		pthread_mutex_destroy(&m_mutex);
	}
	// 获取互斥锁
	bool Lock()
	{
		return pthread_mutex_lock(&m_mutex) == 0;
	}
	// 释放互斥锁
	bool Unlock()
	{
		return pthread_mutex_unlock(&m_mutex) == 0;
	}

private:
	pthread_mutex_t m_mutex;
};

#endif // LOCKER_H