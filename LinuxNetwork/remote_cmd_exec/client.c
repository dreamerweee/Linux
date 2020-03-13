/*
* 连接到服务器，然后输入命令，等待服务器回送消息
* 支持命令： pwd ls cd quit
*/

#include "util.h"

#define KEEPALIVE_TIME 30
#define KEEPALIVE_COUNT 3
#define KEEPALIVE_INTERVAL 3

void SendPing(int fd)
{
    Message msg;
    msg.msg_len = htonl(1);
    msg.msg_type = htonl(MSG_TYPE_PING);
    int ret = send(fd, (char *)&msg, sizeof(msg.msg_len) + sizeof(msg.msg_type), 0);
    if (ret < 0) {
        ErrorExit("SendPing send");
    }
}

void RecvMsg(int fd)
{
    Message msg;
    // 读取消息长度
    int ret = read(fd, &msg.msg_len, sizeof(msg.msg_len));
    if (ret != sizeof(msg.msg_len)) {
        ErrorExit("RecvMsg read length not enough");
    }
    msg.msg_len = ntohl(msg.msg_len);
    // 读取类型
    ret = read(fd, &msg.msg_type, sizeof(msg.msg_type));
    if (ret != sizeof(msg.msg_type)) {
        ErrorExit("RecvMsg read type length error");
    }
    msg.msg_type = ntohl(msg.msg_type);

    ret = ReadSizeN(fd, msg.msg, msg.msg_len);
    if (ret != msg.msg_len) {
        ErrorExit("RecvMsg read msg length error");
    }

    printf("received server msg: %s len: %d\n", msg.msg, strlen(msg.msg));
}

int SendMsg(int fd, int conn_fd)
{
    Message msg;
    int len = read(fd, msg.msg, MAX_LINE - 1);
    if (len < 0) {
        ErrorExit("SendMsg read input");
    }
    msg.msg[len] = '\0';
    if (msg.msg[len - 1] == '\n') {
        msg.msg[len - 1] = '\0';
    }
    if (strlen(msg.msg) == 0) {
        return 0;
    }
    // 退出
    if (strncmp(msg.msg, "quit", 4) == 0) {
        printf("client quit.\n");
        return -1;
    }
    // 发送请求给服务器
    msg.msg_len = htonl(strlen(msg.msg));
    msg.msg_type = htonl(MSG_TYPE_CONTENT);    
    int send_len = sizeof(msg.msg_len) + sizeof(msg.msg_type) + strlen(msg.msg);
    if (send(conn_fd, &msg, send_len, 0) < 0) {
        ErrorExit("SendMsg send");
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int conn_fd = TcpClient(argv[1], argv[2]);

    fd_set init_read_set;
    fd_set read_set;
    FD_ZERO(&init_read_set);
    FD_SET(conn_fd, &init_read_set);
    FD_SET(STDIN_FILENO, &init_read_set);

    struct timeval tv;
    tv.tv_sec = KEEPALIVE_TIME;
    tv.tv_usec = 0;

    int timeout_cnt = 0;
    while (1) {
        read_set = init_read_set;
        int ret = select(conn_fd + 1, &read_set, NULL, NULL, &tv);
        if (ret < 0) {
            ErrorExit("select");
        }
        // 时间到，检测和服务器的连接是否正常
        if (ret == 0) {
            if (++timeout_cnt > KEEPALIVE_COUNT) {
                fprintf(stderr, "connect failed timeout.\n");
                exit(1);
            }
            SendPing(conn_fd);
            tv.tv_sec = KEEPALIVE_INTERVAL;
            continue;
        }

        // 处理服务器消息
        if (FD_ISSET(conn_fd, &read_set)) {
            RecvMsg(conn_fd);
            timeout_cnt = 0;
        }

        // 处理用户输入消息
        if (FD_ISSET(STDIN_FILENO, &read_set)) {
            if (SendMsg(STDIN_FILENO, conn_fd) < 0) {
                break;
            }
        }
        tv.tv_sec = KEEPALIVE_TIME;
    }
    close(conn_fd);
    return 0;
}
