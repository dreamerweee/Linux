#include "util.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int sock_fd = Socket(AF_INET, SOCK_DGRAM);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(sock_fd, &serv_addr, sizeof(serv_addr));

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    ssize_t recv_len;
    char buffer[1024];
    while (1) {
        fprintf(stdout, "begin recv\n");
        recv_len = recvfrom(sock_fd, buffer, 1023, 0, (struct sockaddr*)&client_addr, &client_len);
        buffer[recv_len] = 0;
        fprintf(stdout, "received %d bytes: %s\n", recv_len, buffer);

        char send_msg[1024];
        snprintf(send_msg, 1023, "Hi, %s", buffer);
        sendto(sock_fd, send_msg, strlen(send_msg), 0, (struct sockaddr*)&client_addr, client_len);
    }

    close(sock_fd);
    return 0;
}