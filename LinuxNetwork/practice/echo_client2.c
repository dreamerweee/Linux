#include "util.h"
#include <sys/select.h>

void SendData(int fd)
{
    char *buffer = (char*)malloc(kMaxBufferSize + 1);
    memset(buffer, 'a', kMaxBufferSize);
    buffer[kMaxBufferSize] = '\0';

    ssize_t left = strlen(buffer);
    fprintf(stdout, "send data lenght: %d\n", left);
    const char *cp = buffer;
    while (left > 0) {
        ssize_t send_len = send(fd, cp, left, 0);
        fprintf(stdout, "send into buffer %ld\n", send_len);
        if (send_len == -1) {
            if (errno == EAGAIN) {
                sleep(1);
                continue;
            }
            perror("send");
            exit(1);
        }
        cp += send_len;
        left -= send_len;
    }
}

void DataHandler(int in_fd, int out_fd)
{
    char recv_msg[1024];
    ssize_t recv_len = read(in_fd, recv_msg, 1023);
    if (recv_len < 0) {
        perror("read");
        exit(1);
    } else if (recv_len == 0) {
        fprintf(stdout, "read finish in_fd[%d] out_fd[%d]\n", in_fd, out_fd);
        exit(1);
    } else {
        recv_msg[recv_len] = 0;
        if (strncmp(recv_msg, "close", 5) == 0) {
            close(out_fd);
            sleep(10);
        } else if (strncmp(recv_msg, "shutdown", 8) == 0) {
            shutdown(out_fd, SHUT_WR);
            sleep(10);
        } else {
            write(out_fd, recv_msg, strlen(recv_msg));
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int conn_fd = Socket(AF_INET, SOCK_STREAM);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) != 1) {
        perror("inet_pton");
        exit(1);
    }
    Connect(conn_fd, &serv_addr, sizeof(serv_addr));
    // SetNonblocking(conn_fd);

    fd_set read_set, init_set;
    FD_ZERO(&read_set);
    FD_SET(0, &read_set);
    FD_SET(conn_fd, &read_set);
    int max_fd = conn_fd + 1;

    while (1) {
        init_set = read_set;
        int ret = select(max_fd, &init_set, NULL, NULL, NULL);
        if (ret <= 0) {
            perror("select");
            break;
        } else {
            // 标准输入
            if (FD_ISSET(0, &init_set)) {
                DataHandler(0, conn_fd);
            }
            // 套接字
            if (FD_ISSET(conn_fd, &init_set)) {
                DataHandler(conn_fd, STDOUT_FILENO);
            }
        }
    }
    close(conn_fd);

    return 0;
}