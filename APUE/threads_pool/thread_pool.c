#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static int task_queue_is_empty(ThreadPool *tpool)
{
    return tpool->task_head == tpool->task_tail;
}

static int task_queue_is_full(ThreadPool *tpool)
{
    return (tpool->task_tail + 1) % tpool->max_task_count == tpool->task_head;
}

static void* Worker(void *arg)
{
    ThreadPool *tpool = (ThreadPool*)arg;
    ThreadTask task;

    while (1) {
        pthread_mutex_lock(&tpool->task_mutex);
        while (!tpool->shutdown && task_queue_is_empty(tpool)) {
            pthread_cond_wait(&tpool->not_empty_cond, &tpool->task_mutex);
        }

        if (tpool->shutdown) {
            pthread_mutex_unlock(&tpool->task_mutex);
            printf("thread id[0x%x] exit\n", (unsigned)pthread_self());
            pthread_exit(NULL);
        }
        task = tpool->task[tpool->task_head];
        tpool->task_head = (tpool->task_head + 1) % tpool->max_task_count;
        // 通知可以添加新任务
        pthread_cond_broadcast(&tpool->not_full_cond);

        pthread_mutex_unlock(&tpool->task_mutex);

        // 执行任务
        task.func(task.arg);
    }
    return NULL;
}

// 创建线程池
ThreadPool* ThreadPoolCreate(size_t threads_count, size_t task_count)
{
    ThreadPool *tpool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (!tpool) {
        perror("ThreadPoolCreate malloc");
        exit(1);
    }

    // 数据初始化
    tpool->shutdown = 0;
    tpool->max_threads_count = threads_count;
    tpool->threads = (pthread_t*)malloc(sizeof(pthread_t) * threads_count);
    assert(tpool->threads);
    tpool->max_task_count = task_count;
    tpool->task = (ThreadTask*)malloc(sizeof(ThreadTask) * task_count);
    assert(tpool->task);
    tpool->task_head = 0;
    tpool->task_tail = 0;
    pthread_mutex_init(&tpool->task_mutex, NULL);
    pthread_cond_init(&tpool->not_empty_cond, NULL);
    pthread_cond_init(&tpool->not_full_cond, NULL);

    // 创建threads_count个线程
    int i = 0;
    for (; i < threads_count; ++i) {
        pthread_create(&tpool->threads[i], NULL, Worker, (void*)tpool);
    }

    return tpool;
}

// 往线程池增加任务
void ThreadPoolAddTask(ThreadPool *tpool, void* (*func)(void*), void *arg)
{
    ThreadTask task;
    task.func = func;
    task.arg = arg;

    pthread_mutex_lock(&tpool->task_mutex);
    while (task_queue_is_full(tpool)) {
        printf("ThreadPoolAddTask: task queue had full\n");
        pthread_cond_wait(&tpool->not_full_cond, &tpool->task_mutex);
    }

    // 添加任务
    tpool->task[tpool->task_tail] = task;
    tpool->task_tail = (tpool->task_tail + 1) % tpool->max_task_count;
    printf("ThreadPool task index head[%d] tail[%d]\n", tpool->task_head, tpool->task_tail);
    //通知工作线程
    pthread_cond_signal(&tpool->not_empty_cond);
    pthread_mutex_unlock(&tpool->task_mutex);
}

// 销毁线程池
void ThreadPoolDestroy(ThreadPool *tpool)
{
    if (tpool->shutdown) {
        return;
    }
    tpool->shutdown = 1;

    // 唤醒所有线程，终止任务
    pthread_mutex_lock(&tpool->task_mutex);
    pthread_cond_broadcast(&tpool->not_empty_cond);
    pthread_mutex_unlock(&tpool->task_mutex);

    // 等待所有线程终止
    int i = 0;
    for (; i < tpool->max_threads_count; ++i) {
        pthread_join(tpool->threads[i], NULL);
    }
    // 释放线程数组的内存
    free(tpool->threads);

    // 释放任务数组内存
    free(tpool->task);

    // 销毁锁和条件变量
    pthread_mutex_destroy(&tpool->task_mutex);
    pthread_cond_destroy(&tpool->not_empty_cond);
    pthread_cond_destroy(&tpool->not_full_cond);
    
    free(tpool);
}
