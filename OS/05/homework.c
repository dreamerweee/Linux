#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

pid_t Fork()
{
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "fork falied");
    exit(1);
  }
  return pid; 
}

void Work2()
{
  int fd = open("homework.txt", O_CREAT | O_RDWR, S_IRWXU);
  if (fd < 0) {
    fprintf(stderr, "open failed\n");
    exit(1);
  }
  pid_t pid = Fork();
  if (pid == 0) {
    fprintf(stdout, "I am child.\n");
    int i = 0;
    while (i < 10000) {
      ++i;
      write(fd, "child ", 6);
    }
  } else {
    fprintf(stdout, "I am parent\n");
    int i = 0;
    while (i < 10000) {
      ++i;
      write(fd, "parent ", 7);
    }
  }
  close(fd);
}

void Work7()
{
  pid_t pid = Fork();
  if (pid == 0) {
    close(STDOUT_FILENO);
    printf("hello child\n");
  } else {
    printf("hello parent\n");
  }
}

void Work8()
{
  int fd[2];
  if (pipe(fd) != 0)  {
    fprintf(stderr, "pipe failed\n");
    exit(1);
  }
  pid_t pid = Fork();
  if (pid == 0) { // 第一个子进程读管道
    close(fd[1]);
    char buff[1024];
    ssize_t count;
    if ((count = read(fd[0], buff, 1023)) < 0) {
      fprintf(stderr, "read failed\n");
      exit(1);
    }
    buff[count] = 0;
    printf("child[%d] read: %s\n", getpid(), buff);
    close(fd[0]);
    exit(0);
  } else {
    pid = Fork();
    if (pid == 0) {  // 第二个子进程写
      close(fd[0]);
      printf("child[%d] write: hello\n", getpid());
      write(fd[1], "hello", 5);
      close(fd[1]);
      exit(0);
    } else {
      close(fd[0]);
      close(fd[1]);

      int wc = wait(NULL);
      printf("parent wait: %d\n", wc);
    }
  }
}

int main(int argc, char *argv[])
{
  // Work7();
  Work8();
  return 0;
}
