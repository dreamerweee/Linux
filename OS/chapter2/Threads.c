#include <stdio.h>
#include <stdlib.h>
#include "Common.h"
#include "CommonThreads.h"

volatile int counter = 0;
int loops;

void* Worker(void *args)
{
  int i;
  for (i = 0; i < loops; ++i) {
    ++counter;
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: Threads <loops>\n");
    exit(1);
  }

  loops = atoi(argv[1]);
  pthread_t thread1, thread2;
  printf("Initial value: %d\n", counter);
  PthreadCreate(&thread1, NULL, Worker, NULL);
  PthreadCreate(&thread2, NULL, Worker, NULL);

  PthreadJoin(thread1, NULL);
  PthreadJoin(thread2, NULL);

  printf("Final value: %d\n", counter);
  return 0;
}