#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
  printf("hello world pid[%d]\n", (int)getpid());
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "fork failed\n");
    exit(1);
  } else if (pid == 0) {
    printf("hello, I am child pid[%d]\n", (int)getpid());
    char *my_args[3];
    my_args[0] = strdup("wc");
    my_args[1] = strdup("fork1.c");
    my_args[2] = NULL;
    execvp(my_args[0], my_args);
    printf("this shouldn't print out\n");
  } else {
    int wc = wait(NULL);
    printf("hello, I am parent of %d wc[%d] pid[%d]\n", (int)pid, wc, (int)getpid());
  }
  return 0;
}