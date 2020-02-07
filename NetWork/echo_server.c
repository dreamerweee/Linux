#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in srv_addr;
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(atoi(argv[1]));
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if (ret < 0) {
        perror("bind");
        exit(1);
    }

    ret = listen(listen_fd, 5);
    if (ret < 0) {
        perror("listen");
        exit(1);
    }
    printf("echo_server begin accept:\n");
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    char str[INET_ADDRSTRLEN];

    ssize_t len;
    char msg[1024];
    
    while (1) {
        int conn_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if (conn_fd < 0) {
            perror("accept");
        } else {
            if (inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)) == NULL) {
                perror("inet_ntop");
            } else {
                printf("connect from ip[%s] port[%d]\n", str, ntohs(cli_addr.sin_port));
                while ((len = read(conn_fd, msg, 1023)) > 0) {
                    msg[len] = '\0';
                    for (int i = 0; i < len; ++i) {
                        msg[i] = toupper(msg[i]);
                    }
                    write(conn_fd, msg, strlen(msg));
                }
            }

            close(conn_fd);
        }
    }

    close(listen_fd);
    return 0;
}
