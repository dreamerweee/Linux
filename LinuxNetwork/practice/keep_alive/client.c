#include "message.h"
#include "../util.h"

const int kMaxBuffer = 1024;
enum {
  KEEP_ALIVE_TIME = 10,
  KEEP_ALIVE_INTERVAL = 3,
  KEEP_ALIVE_PROBETIMES = 3
};

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
    exit(1);
  }
  
  int conn_fd = Socket(AF_INET, SOCK_STREAM);

  struct sockaddr_in svr_addr;
  InitSocketAddr(&svr_addr, AF_INET, argv[1], argv[2]);

  Connect(conn_fd, &svr_addr, sizeof(svr_addr));

  // 使用select的定时功能来判断连接是否有效
  struct timeval tv;
  tv.tv_sec = KEEP_ALIVE_TIME;
  tv.tv_usec = 0;

  fd_set init_read_set;
  fd_set read_set;
  FD_ZERO(&init_read_set);
  FD_SET(conn_fd, &init_read_set);

  Message msg;

  int heart_beats = 0;
  char recv_msg[kMaxBuffer + 1];
  ssize_t recv_len;

  while (1) {
    read_set = init_read_set;
    int ret = select(conn_fd + 1, &read_set, NULL, NULL, &tv);
    if (ret < 0) {
      perror("select");
      break;
    }
    // 超时时间到，心跳检测
    if (ret == 0) {
      if (++heart_beats >= KEEP_ALIVE_PROBETIMES) {
        fprintf(stderr, "connect timeout close\n");
        break;
      }
      printf("send heart beat #%d\n", heart_beats);
      msg.type = htonl(MSG_PING);
      ret = send(conn_fd, (char *)&msg, sizeof(msg), 0);
      if (ret < 0) {
        perror("send");
        break;
      }
      tv.tv_sec = KEEP_ALIVE_INTERVAL;
      continue;
    }
    if (FD_ISSET(conn_fd, &read_set)) {
      recv_len = read(conn_fd, recv_msg, kMaxBuffer);
      if (recv_len < 0) {
        perror("read");
        break;
      } else if (recv_len == 0) {
        printf("server terminated.\n");
        break;
      }
      printf("received heart beat, make heart beat to 0.\n");
      heart_beats = 0;
      tv.tv_sec = KEEP_ALIVE_TIME;
    }
  }
  close(conn_fd);

  return 0;
}