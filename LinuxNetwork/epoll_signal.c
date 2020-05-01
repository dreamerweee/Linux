#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>

int pipe_fd[2];

void SignalHandler(int sig)
{
    printf("receive signal[%d]\n", sig);
    int ret = send(pipe_fd[1], (char *)&sig, 1, 0);
    printf("send signal len:%d %d\n", ret, sizeof(sig));
}

void AddSignal(int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SignalHandler;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    if (sigaction(sig, &sa, NULL) != 0) {
        printf("sigaction failed.\n");
        exit(-1);
    }
}

void EpollAddFD(int epoll_fd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
        printf("epoll_ctl failed.\n");
        exit(-1);
    }
}

int main()
{
    int epoll_fd = epoll_create(5);
    if (epoll_fd < 0) {
        printf("epoll_create failed.\n");
        exit(-1);
    }
    
    int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, pipe_fd);
    if (ret < 0) {
        printf("socketpair failed.\n");
        exit(-1);
    }
    EpollAddFD(epoll_fd, pipe_fd[0]);

    // signal
    AddSignal(SIGPIPE);
    AddSignal(SIGHUP);

    struct epoll_event events[2];

    while (1) {
        int cnt = epoll_wait(epoll_fd, events, 2, -1);
        if (cnt < 0 && errno != EINTR) {
            printf("epoll_wait failed.[%s]\n", strerror(errno));
            exit(-1);
        }
        printf("epoll_wait return cnt = %d\n", cnt);
        int i;
        for (i = 0; i < cnt; ++i) {
            int fd = events[i].data.fd;
            char buff[1024];
            ret = recv(fd, buff, sizeof(buff), 0);
            if (ret <= 0) {
                printf("recv failed.\n");
                continue;
            }
            int j = 0;
            for (; j < ret; ++j) {
                switch (buff[j]) {
                    case SIGPIPE:
                        printf("-==========================\n");
                        break;
                }
            }
        }
    }

    return 0;
}