#include <time.h>
#include <signal.h>
#include "util.h"

void ReadData(int fd)
{
    char buffer[1024];
    int time = 0;
    ssize_t n;
    while (1) {
        fprintf(stdout, "block in read\n");
        if ((n = ReadCount(fd, buffer, 1024)) == 0) {
            return;
        }
        ++time;
        fprintf(stdout, "1k read for %d\n", time);
        usleep(1000);
    }
}

void SignalHandler(int sig_no)
{
    fprintf(stdout, "receive signal[%d]\n", sig_no);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    if (atoi(argv[1]) == 0) {
        fprintf(stderr, "error port\n");
        exit(1);
    }

    int listen_fd = Socket(AF_INET, SOCK_STREAM);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listen_fd, &serv_addr, sizeof(serv_addr));
    Listen(listen_fd);

    signal(SIGPIPE, SignalHandler);

    int conn_fd = accept(listen_fd, NULL, NULL);
    if (conn_fd == -1) {
        perror("accept");
        exit(1);
    }
    
    char recv_msg[1024];
    ssize_t recv_len;
    while (1) {
        recv_len = read(conn_fd, recv_msg, 1023);
        if (recv_len < 0) {
            perror("read");
            break;
        } else if (recv_len == 0) {
            printf("close by peer.\n");
            break;
        } else {
            recv_msg[recv_len] = 0;
            printf("received %d bytes msg: %s\n", recv_len, recv_msg);
            sleep(5);
            if (send(conn_fd, recv_msg, strlen(recv_msg), 0) < 0) {
                perror("send");
                break;
            }
        }
    }
    close(conn_fd);
    close(listen_fd);
    return 0;
}
