#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/un.h>

const int kMaxBufferSize = 1024; // udp 数据报可携带的最大字节数 65535 - 20 - 8

ssize_t ReadCount(int fd, void *buffer, size_t count)
{
    void *buffer_head = buffer;
    size_t len = count;
    while (len > 0) {
        ssize_t ret = read(fd, buffer_head, len);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        } else if (ret == 0) {
            break;
        }
        len -= ret;
        buffer_head = buffer_head + ret;
    }
    return (count - len);
}

int Socket(int domain, int type)
{
    int fd = socket(domain, type, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }
    return fd;
}

void Bind(int fd, void *addr, socklen_t addr_len)
{
    if (bind(fd, (struct sockaddr*)addr, addr_len) < 0) {
        perror("bind");
        exit(1);
    }
}

void Listen(int fd)
{
    if (listen(fd, 5) < 0) {
        perror("listen");
        exit(1);
    }
}

void Connect(int fd, void *addr, socklen_t addr_len)
{
    if (connect(fd, (struct sockaddr*)addr, addr_len) < 0) {
        perror("connect");
        exit(1);
    }
}

int SetNonblocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}

void InitSocketAddr(struct sockaddr_in *addr, int domain, const char *ip, const char *port)
{
    addr->sin_family = domain;
    addr->sin_port = htons(atoi(port));
    if (inet_pton(domain, ip, &addr->sin_addr) != 1) {
        perror("inet_pton");
        exit(1);
    }
}

#endif  // UTILS_H