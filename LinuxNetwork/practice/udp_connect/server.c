#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
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
  svr_addr.sin_port = htons(atoi(argv[1]));
  svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) {
    perror("bind");
    exit(1);
  }

  char recv_msg[1024];

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  ssize_t recv_len = recvfrom(sock_fd, recv_msg, 1023, 0, (struct sockaddr *)&client_addr, &client_len);
  if (recv_len < 0) {
    perror("recvfrom");
    exit(1);
  }
  recv_msg[recv_len] = 0;
  printf("received %d bytes: %s\n", recv_len, recv_msg);

  if (connect(sock_fd, (struct sockaddr *)&client_addr, client_len) < 0) {
    perror("connect");
    exit(1);
  }

  while (strncmp(recv_msg, "goodbye", 7) != 0) {
    char send_msg[1024];
    sprintf(send_msg, "Hi, %s", recv_msg);

    ssize_t ret = send(sock_fd, send_msg, strlen(send_msg), 0);
    if (ret < 0) {
      perror("send");
      exit(1);
    }
    printf("send bytes: %zu\n", ret);
    recv_len = recv(sock_fd, recv_msg, 1023, 0);
    if (recv_len < 0) {
      perror("recv");
      exit(1);
    }
    recv_msg[recv_len] = 0;
  }
  close(sock_fd);

  return 0;
}