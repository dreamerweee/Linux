#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  int fd = open("homework.txt", O_CREAT | O_RDWR, S_IRWXU);
  if (fd < 0) {
    fprintf(stderr, "open failed\n");
    exit(1);
  }
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "fork falied");
    close(fd);
    exit(1);
  } else if (pid == 0) {
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
  return 0;
}
