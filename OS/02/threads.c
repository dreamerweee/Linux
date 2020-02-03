#include <stdio.h>
#include <stdlib.h>
#include "common_threads.h"

volatile int g_counter = 0;
int g_loops;

void* Worker(void *arg)
{
    int i = 0;
    for (; i < g_loops; ++i) {
        g_counter++;  // ++g_counter
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: threads <value>\n");
        exit(1);
    }

    g_loops = atoi(argv[1]);
    pthread_t tid1, tid2;
    printf("before work, counter = %d\n", g_counter);
    PthreadCreate(&tid1, NULL, Worker, NULL);
    PthreadCreate(&tid2, NULL, Worker, NULL);
    PthreadJoin(tid1, NULL);
    PthreadJoin(tid2, NULL);
    printf("after work, counter = %d\n", g_counter);
    return 0;
}