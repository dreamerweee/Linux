#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* Func(void *arg)
{
    printf("enter func arg[%d]\n", (int)arg);
    // sleep(1);
    printf("finish func arg[%d]\n", (int)arg);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: %s threads_count task_count\n", argv[0]);
        exit(1);
    }

    size_t threads_count = atoi(argv[1]);
    size_t task_count = atoi(argv[2]);

    ThreadPool *tpool = ThreadPoolCreate(threads_count, task_count);
    int i = 0;
    for (; i < task_count * 10; ++i) {
        ThreadPoolAddTask(tpool, Func, (void*)i);
    }

    sleep(2);
    printf("begin destroy thread pool------------------------\n");
    ThreadPoolDestroy(tpool);
    return 0;
}