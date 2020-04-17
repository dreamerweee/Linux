#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

bool run_flag = true;

int Daemon()
{
    pid_t pid = fork();
    if (pid == -1) {
        exit(-1);
    }
    if (pid > 0) {
        exit(0);
    }

    if (setsid() == -1) {
        exit(-1);
    }

    pid = fork();
    if (pid == -1) {
        exit(-1);
    }
    if (pid > 0) {
        exit(0);
    }

    chdir("/");
    umask(0);
    int i = 0;
    for (; i < 3; ++i) {
        close(i);
    }
    return 0;
}

void Handler(int signo)
{
    printf("receive signal[%d] exit.\n", signo);
    run_flag = false;
}

int main(void)
{
    Daemon();
    struct sigaction sa;
    sa.sa_handler = Handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    int ret = sigaction(SIGQUIT, &sa, NULL);
    if (ret != 0) {
        printf("sigaction failed.\n");
        exit(-1);
    }

    int fd = open("/home/weee/daemon.log", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
    if (fd < 0) {
        printf("open failed.\n");
        exit(-1);
    }
    time_t t;
    while (run_flag) {
        t = time(NULL);
        char *buff = asctime(localtime(&t));
        write(fd, buff, strlen(buff));
        sleep(60);
    }

    return 0;
}
