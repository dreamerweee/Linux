#ifndef __common_threads_h__
#define __common_threads_h__

#include <pthread.h>
#include <assert.h>
// #include <sched.h>
#include <semaphore.h>

#define PthreadCreate(thread, attr, start_routine, arg) assert(pthread_create(thread, attr, start_routine, arg) == 0);
#define PthreadJoin(thread, value_ptr) assert(pthread_join(thread, value_ptr) == 0);
#define PthreadMutexLock(m) assert(pthread_mutex_lock(m) == 0);
#define PthreadMutexUnlock(m) assert(pthread_mutex_unlock(m) == 0);
#define PthreadCondSignal(cond) assert(pthread_cond_signal(cond) == 0);
#define PthreadCondWait(cond, mutex) assert(pthread_cond_wait(cond, mutex) == 0);

#define MutexInit(m) assert(pthread_mutex_init(m, NULL) == 0);
#define MutexLock(m) assert(pthread_mutex_lock(m) == 0);
#define MutexUnlock(m) assert(pthread_mutex_unlock(m) == 0);
#define CondInit(cond) assert(pthread_cond_init(cond, NULL) == 0);
#define CondSignal(cond) assert(pthread_cond_signal(cond) == 0);
#define CondWait(cond, mutex) assert(pthread_cond_wait(cond, mutex) == 0);

#define SemInit(sem, value) assert(sem_init(sem, 0, value) == 0);
#define SemWait(sem) assert(sem_wait(sem) == 0);
#define SemPost(sem) assert(sem_post(sem) == 0);

#endif // __common_threads_h__

