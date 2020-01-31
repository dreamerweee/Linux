#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

void SigHandler(int sig_num)
{
    if (sig_num != SIGUSR1) {
        printf("receive other signal[%d]\n", sig_num);
        return;
    }
    printf("received signal.\n");
    int fd = open("tmp", O_RDONLY);
    if (fd < 0) {
        perror("open:");
        return;
    }
    char buff[1024];
    int n = read(fd, buff, 1024);
    if (n < 0) {
        perror("read:");
        return;
    }
    int i;
    for (i = 0; i < n; ++i) {
        putchar(toupper(buff[i]));
    }
    close(fd);
}

int main()
{
    printf("my pid[%d]\n", getpid());

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SigHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction:");
        return -1;
    }

    while (1) {
        pause();  // 进入睡眠
    }
    return 0;
}