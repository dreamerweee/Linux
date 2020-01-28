#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

void SigChild(int sig_num)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, 0)) < 0 ) {
        printf("SigChild waitpid error\n");
        exit(-1);
    }
    sleep(1);
}

int main()
{
    printf("parent pid[%d]\n", getpid());
    signal(SIGCHLD, SigChild);
    int n = 10;
    pid_t pid;
    while (n-- > 0) {
        pid = fork();
        if (pid < 0) {
            printf("fork error\n");
            return -1;
        }
        if (pid == 0) {
            printf("child pid[%d]\n", getpid());
            return 0;
        }
    }

    while (1) {
        write(STDOUT_FILENO, ".", 1);
        sleep(10);
    }

    return 0;
}
