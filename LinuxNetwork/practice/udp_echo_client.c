#include "util.h"

// extern const int kMaxBufferSize;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int sock_fd = Socket(AF_INET, SOCK_DGRAM);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) != 1 ) {
        perror("inet_pton");
        exit(1);
    }

    // char *buffer = (char *)malloc(kMaxBufferSize + 1);
    // if (!buffer) {
    //     perror("malloc");
    //     exit(1);
    // }
    // memset(buffer, 'a', kMaxBufferSize);
    // buffer[kMaxBufferSize] = '\0';
    // fprintf(stdout, "begin send\n");
    // if (sendto(sock_fd, buffer, kMaxBufferSize + 1, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    //     perror("sendto");
    //     exit(1);
    // }
    // struct sockaddr *from_addr;
    // socklen_t from_len;
    // fprintf(stdout, "begin recv\n");
    // while (1) {
    //     char recv_msg[1024];
    //     ssize_t recv_len = recvfrom(sock_fd, recv_msg, 1023, 0, from_addr, &from_len);
    //     if (recv_len < 0) {
    //         perror("recvfrom");
    //         exit(1);
    //     }
    //     recv_msg[recv_len] = 0;
    //     fputs(recv_msg, stdout);
    //     fputs("\n", stdout);
    // }

    socklen_t serv_len = sizeof(serv_addr);
    char send_msg[1024], recv_msg[1024];
    socklen_t from_len;
    ssize_t recv_len, send_len;
    struct sockaddr_in from_addr;
    while (fgets(send_msg, 1024, stdin) != NULL) {
        int i = strlen(send_msg);
        if (send_msg[i - 1] == '\n') {
            send_msg[i - 1] = '\0';
        }
        fprintf(stdout, "begin send %s\n", send_msg);
        send_len = sendto(sock_fd, send_msg, strlen(send_msg), 0, (struct sockaddr*)&serv_addr, serv_len);
        if (send_len < 0) {
            perror("sendto");
            exit(1);
        }
        fprintf(stdout, "begin recv\n");
        // from_len = 0;
        recv_len = recvfrom(sock_fd, recv_msg, 1023, 0, (struct sockaddr*)&from_addr, &from_len);
        if (recv_len < 0) {
            perror("recvfrom");
            exit(1);
        }
        recv_msg[recv_len] = '\0';
        fputs(recv_msg, stdout);
        fputs("\n", stdout);
    }

    close(sock_fd);
    return 0;
}