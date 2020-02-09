#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

// 0：不加锁  1：互斥锁  2：读写锁
int g_lock = 0;

typedef struct {
    int m_count;
    pthread_mutex_t m_mutex;
    pthread_rwlock_t m_rwlock;
} Goods;

Goods g_goods = {1000};

// 查询线程
void* QueryWorker(void *arg)
{
    if (g_lock == 1) {
        pthread_mutex_lock(&g_goods.m_mutex);
    } else if (g_lock == 2) {
        pthread_rwlock_rdlock(&g_goods.m_rwlock);
    }

    int remain = g_goods.m_count;
    sleep(1);  // 模拟耗时
    printf("%03d Query: %d\n", (int)arg, remain);

    if (g_lock == 1) {
        pthread_mutex_unlock(&g_goods.m_mutex);
    } else if (g_lock == 2) {
        pthread_rwlock_unlock(&g_goods.m_rwlock);
    }

    return NULL;
}

// 抢购线程
void* BuyWorker(void *arg)
{
    if (g_lock == 1) {
        pthread_mutex_lock(&g_goods.m_mutex);
    } else if (g_lock == 2) {
        pthread_rwlock_wrlock(&g_goods.m_rwlock);
    }

    int remain = g_goods.m_count;
    remain -= 10;  // 一次买10个
    sleep(1);  // 模拟耗时
    g_goods.m_count = remain;
    printf("%03d buy goods\n", (int)arg);

    if (g_lock == 1) {
        pthread_mutex_unlock(&g_goods.m_mutex);
    } else if (g_lock == 2) {
        pthread_rwlock_unlock(&g_goods.m_rwlock);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s <0|1|2>\n", argv[0]);
        exit(1);
    }

    g_lock = atoi(argv[1]);
    pthread_t tids[100];

    // 初始化锁
    pthread_mutex_init(&g_goods.m_mutex, NULL);
    pthread_rwlock_init(&g_goods.m_rwlock, NULL);

    // 创建线程进行查询和抢购
    int i = 0;
    for (; i < 100; ++i) {
        if (i % 10 == 0) {
            pthread_create(&tids[i], NULL, BuyWorker, (void*)i);
        } else {
            pthread_create(&tids[i], NULL, QueryWorker, (void*)i);
        }
    }

    for (i = 0; i < 100; ++i) {
        pthread_join(tids[i], NULL);
    }

    // 销毁锁
    pthread_mutex_destroy(&g_goods.m_mutex);
    pthread_rwlock_destroy(&g_goods.m_rwlock);

    printf("remain goods count = %d\n", g_goods.m_count);

    return 0;
}
