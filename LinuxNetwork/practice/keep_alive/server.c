#include "message.h"
#include "../util.h"

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "usage: %s <port> <sleep time>\n", argv[0]);
    exit(1);
  }
  int sleep_time = atoi(argv[2]);

  int listen_fd = Socket(AF_INET, SOCK_STREAM);

  struct sockaddr_in svr_addr;
  memset(&svr_addr, 0, sizeof(svr_addr));
  svr_addr.sin_family = AF_INET;
  svr_addr.sin_port = htons(atoi(argv[1]));
  svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  Bind(listen_fd, &svr_addr, sizeof(svr_addr));
  Listen(listen_fd);

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
  if (conn_fd < 0) {
    perror("accept");
    exit(1);
  }

  Message msg;
  ssize_t recv_len;
  while (1) {
    recv_len = read(conn_fd, &msg, sizeof(Message));
    if (recv_len < 0) {
      perror("read");
      break;
    } else if (recv_len == 0) {
      printf("client closed.\n");
      break;
    }
    printf("received %d bytes\n", recv_len);
    switch(ntohl(msg.type)) {
      case MSG_PING:
      {
        Message response;
        response.type = MSG_PONG;
        sleep(sleep_time);
        if (send(conn_fd, (char *)&response, sizeof(response), 0) < 0) {
          perror("send");
          exit(1);
        }
        break;
      }
      default:
        fprintf(stderr, "unknown message type.\n");
    }
  }
  close(conn_fd);
  close(listen_fd);

  return 0;
}