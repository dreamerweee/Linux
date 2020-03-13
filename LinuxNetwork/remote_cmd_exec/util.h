#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_LINE 128

typedef struct {
    uint32_t msg_len;
    uint32_t msg_type;
    char msg[MAX_LINE];
} Message;

#define MSG_TYPE_PING 1
#define MSG_TYPE_CONTENT 2

void ErrorExit(const char *msg)
{
    perror(msg);
    exit(1);
}

// 创建客户端套接字
int TcpClient(const char *ip, const char *port)
{
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd < 0)  {
        ErrorExit("socket");
    }

    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(atoi(port));
    if (inet_pton(AF_INET, ip, &svr_addr.sin_addr) != 1) {
        ErrorExit("inet_pton");
    }

    if (connect(conn_fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) {
        ErrorExit("connect");
    }

    return conn_fd;
}

// 创建服务器监听套接字
int TcpServer(const char *port)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        ErrorExit("socket");
    }

    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(atoi(port));
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(listen_fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) {
        ErrorExit("bind");
    }
    if (listen(listen_fd, 5) < 0) {
        ErrorExit("listen");
    }

    return listen_fd;
}

ssize_t ReadSizeN(int fd, char *buffer, ssize_t length)
{
    if (length <= 0 || length >= MAX_LINE) {
        return -1;
    }

    ssize_t left_cnt = length;
    char *buffer_head = buffer;

    while (left_cnt > 0) {
        ssize_t recv_len = read(fd, buffer_head, left_cnt);
        if (recv_len < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            ErrorExit("ReadSizeN read");
        }
        if (recv_len == 0) {
            ErrorExit("ReadSizeN server closed.");
        }
        left_cnt -= recv_len;
        buffer_head += recv_len;
    }
    *buffer_head = '\0';

    return length - left_cnt;
}

#endif  // UTIL_H