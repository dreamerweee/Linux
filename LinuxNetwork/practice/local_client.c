#include "util.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <local_path>\n", argv[0]);
        exit(1);
    }

    int conn_fd = Socket(AF_LOCAL, SOCK_STREAM);
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_LOCAL;
    strcpy(serv_addr.sun_path, argv[1]);

    if (connect(conn_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    char send_msg[1024];
    char recv_msg[1028];
    while (fgets(send_msg, 1024, stdin) != NULL) {
        int len = strlen(send_msg);
        fprintf(stdout, "write: len=%d\n", len);
        if (write(conn_fd, send_msg, len) != len) {
            perror("write");
            break;
        }
        memset(recv_msg, 0, 1028);
        if (read(conn_fd, recv_msg, 1028) <= 0) {
            perror("read");
            break;
        }
        fprintf(stdout, "recv: len=%d, msg=%s\n", strlen(recv_msg), recv_msg);
    }
    close(conn_fd);

    return 0;   
}