#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in srv_addr;
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(atoi(argv[2]));
    int ret = inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);
    if (ret != 1) {
        perror("inet_pton");
        close(conn_fd);
        exit(1);
    }

    ret = connect(conn_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if (ret < 0) {
        perror("connect");
        close(conn_fd);
        exit(1);
    }

    char msg[1024];
    bzero(msg, 1024);
    ssize_t len;
    while ((len = read(STDIN_FILENO, msg, 1023)) > 0) {
        msg[len] = '\0';
        write(conn_fd, msg, strlen(msg));
        if (read(conn_fd, msg, 1023) <= 0) {
            perror("read error");
            break;
        }
        printf("receive server msg: %s\n", msg);
    }
    close(conn_fd);
    return 0;
}
