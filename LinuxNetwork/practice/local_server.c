#include "util.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "%s <local_path>\n", argv[0]);
        exit(1);
    }

    int listen_fd = Socket(AF_LOCAL, SOCK_STREAM);
    struct sockaddr_un serv_addr;
    char *local_path = argv[1];
    unlink(local_path);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_LOCAL;
    strcpy(serv_addr.sun_path, local_path);

    Bind(listen_fd, &serv_addr, sizeof(serv_addr));
    Listen(listen_fd);

    struct sockaddr_un cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int conn_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &cli_len);
    if (conn_fd < 0) {
        perror("accept");
        exit(1);
    }

    char buffer[1024];
    while (1) {
        memset(buffer, 0, 1024);
        if (read(conn_fd, buffer, 1023) <= 0) {
            perror("read");
            break;
        }
        fprintf(stdout, "receive: %s\n", buffer);
        
        // 回写
        char send_msg[1024 + 4];
        memset(send_msg, 0, 1028);
        snprintf(send_msg, 1027, "Hi, %s", buffer);
        write(conn_fd, send_msg, strlen(send_msg));
    }
    close(listen_fd);
    close(conn_fd);

    return 0;
}