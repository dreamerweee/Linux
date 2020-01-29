#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

int main()
{
    time_t now_time = time(NULL);
    pid_t pid;
    int count = 5;
    while (count-- > 0) {
        if ((pid = fork()) < 0) {
            perror("fork:");
            return -1;
        } else if (pid == 0) {
            break;    // 子进程直接退出循环
        }
    }
    
    if (pid == 0) {  // 子进程
        int sleep_time = count * count + 1;
        printf("child pid[%d] before sleep_time[%d]\n", getpid(), sleep_time);
        sleep(sleep_time);
        printf("child pid[%d] after sleep_time[%d]\n", getpid(), sleep_time);
        return sleep_time;
    }

    int status = 0;
    while (1) {
        pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
        if (pid == 0) {
            printf("pid[%d] wait child status change\n", getpid());
        } else if (pid < 0) {
            printf("some error happend\n");
            break;
        } else {
            if (WIFEXITED(status)) {
                printf("child[%d] exit with code[%d]\n", pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("signal child[%d] signal[%d]\n", pid, WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stop child[%d] signal[%d]]n", pid, WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("continued child[%d] \n", pid);
            }
        }
    }

    printf("use time[%d]", time(NULL) - now_time);

    return 0;   
}