#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
    exit(1);
  }

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_in svr_addr;
  memset(&svr_addr, 0, sizeof(svr_addr));
  svr_addr.sin_family = AF_INET;
  svr_addr.sin_port = htons(atoi(argv[2]));
  if (inet_pton(AF_INET, argv[1], &svr_addr.sin_addr) != 1) {
    perror("inet_pton");
    exit(1);
  }

  if (connect(sock_fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) {
    perror("connect");
    exit(1);
  }

  char send_msg[1024], recv_msg[1024];
  ssize_t recv_len;
  while (fgets(send_msg, 1024, stdin) != NULL) {
    int len = strlen(send_msg);
    if (send_msg[len - 1] == '\n') {
      send_msg[len - 1] = 0;
    }
    printf("now send msg: %s\n", send_msg);
    if (send(sock_fd, send_msg, strlen(send_msg), 0) < 0) {
      perror("send");
      exit(1);
    }
    recv_len = recv(sock_fd, recv_msg, 1023, 0);
    if (recv_len < 0) {
      perror("recv");
      exit(1);
    }
    recv_msg[recv_len] = 0;
    fputs(recv_msg, stdout);
    printf("\n");
  }
  close(sock_fd);

  return 0;
}