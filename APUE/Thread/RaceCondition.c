#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_LOOP_VALUE 1000000

int g_count = 0;

static void* Thread(void *param)
{
	int i = 0;
	for (; i < MAX_LOOP_VALUE; ++i) {
		++g_count;
	}
	return NULL;
}

int main()
{
	pthread_t t1, t2;
	pthread_create(&t1, NULL, Thread, NULL);
	pthread_create(&t2, NULL, Thread, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	printf("g_count = %d\n", g_count);
	return 0;
}