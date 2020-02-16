#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "fork failed\n");
    exit(1);
  } else if (pid == 0) {  // child
    close(STDOUT_FILENO);
    open("out.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

    // 执行 wc 命令
    char *wc_args[3];
    wc_args[0] = strdup("wc");
    wc_args[1] = strdup("fork2.c");
    wc_args[2] = NULL;
    execvp(wc_args[0], wc_args);
  } else {
    int wc = wait(NULL);
  }
  return 0;
}