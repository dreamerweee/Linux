#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

// 任务结构
typedef struct ThreadTask {
    void* (*func)(void*);
    void *arg;
} ThreadTask;

// 线程池结构
typedef struct {
    int shutdown;   // 线程池是否关闭: 1 yes  0 no

    size_t max_threads_count;  // 线程的最大数量
    pthread_t *threads;  // 线程数组

    size_t max_task_count;  // 最大任务数
    ThreadTask *task;  // 任务队列
    size_t task_head;  // 当前任务头  task_head == task_tail, 队列空
    size_t task_tail;  // 当前任务尾  (task_tail + 1) % max_task_count == task_head, 队列满

    pthread_mutex_t task_mutex;  // 任务队列的锁
    pthread_cond_t not_empty_cond;  // 条件变量, 队列不为空
    pthread_cond_t not_full_cond;  // 任务队列不为满
} ThreadPool;

// 创建线程池
ThreadPool* ThreadPoolCreate(size_t threads_count, size_t task_count);

// 销毁线程池
void ThreadPoolDestroy(ThreadPool *tpool);

// 往线程池增加任务
void ThreadPoolAddTask(ThreadPool *tpool, void* (*func)(void*), void *arg);

#endif  // THREAD_POOL_H