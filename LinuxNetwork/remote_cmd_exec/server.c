/*
* 处理客户端请求，将结果返回给客户端
*/
#include "util.h"
#include <sys/stat.h>
#include <dirent.h>

void AcceptNew(int fd, fd_set *p_set)
{
    int conn_fd = accept(fd, NULL, NULL);
    if (conn_fd < 0) {
        fprintf(stderr, "AcceptNew accept error:%d\n", errno);
        return;
    }
    FD_SET(conn_fd, p_set);
    printf("accept new client.\n");
}

int SendPing(int fd)
{
    Message msg;
    msg.msg_len = htonl(4);
    msg.msg_type = htonl(MSG_TYPE_PING);
    strncpy(msg.msg, "ping", 4);
    msg.msg[4] = '\0';

    int send_len = sizeof(msg.msg_len) + sizeof(msg.msg_type) + strlen(msg.msg);
    int ret = send(fd, (char *)&msg, send_len, 0);
    if (ret < 0) {
        fprintf(stderr, "SendPing error:%d\n", errno);
        return -1;
    }
    return 0;
}

void CmdLs(char *msg)
{
    char dir[MAX_LINE];
    if (getcwd(dir, MAX_LINE) == NULL) {
        fprintf(stderr, "cmd ls getcwd error: %s\n", strerror(errno));
        strncpy(msg, "ls exec error", MAX_LINE);
        return;
    }
    DIR *p_dir = opendir(dir);
    if (p_dir == NULL) {
        fprintf(stderr, "cmd ls opendir failed:%s", strerror(errno));
        strncpy(msg, "ls exec error", MAX_LINE);
        return;
    }
    char *msg_head = msg;
    struct dirent *p_dirent;
    struct stat stat_buff;
    int send_len = 0;
    while ((p_dirent = readdir(p_dir)) != NULL) {
        if (strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0) {
            continue;
        }
        int len = strlen(p_dirent->d_name);
        if (send_len + len + 2 > MAX_LINE) {  // 2 表示 \n \0的位置
            fprintf(stderr, "cmd ls too long.\n");
            break;
        }
        strncpy(msg_head, p_dirent->d_name, len);
        msg_head += len;
        *msg_head++ = '\n';
        send_len += len + 1;
    }
    *msg_head = '\0';
    closedir(p_dir);
}

void CmdCd(char *msg, const char *recv_msg)
{
    int recv_len = strlen(recv_msg);
    if (recv_len <= 3) {  // "cd "
        fprintf(stderr, "CmdCd cd error:%s\n", strerror(errno));
        strncpy(msg, "not find path", MAX_LINE);
        return;
    }
    char path[MAX_LINE];
    int cp_len = recv_len - 3;
    memcpy(path, recv_msg + 3, cp_len);
    path[cp_len] = '\0';
    if (chdir(path) == -1) {
        sprintf(msg, "cd error:%s", strerror(errno));
    } else {
        sprintf(msg, "cd exec ok.\n");
    }
}

void CmdPwd(char *msg)
{
    if (getcwd(msg, MAX_LINE) == NULL) {
        fprintf(stderr, "getcwd error: %d\n", errno);
        strncpy(msg, "pwd exec error", MAX_LINE);
    }
}

void CmdUnknow(char *msg, const char *recv_msg)
{
    fprintf(stderr, "unknow cmd: %s\n", recv_msg);
    strncpy(msg, "unknow cmd", MAX_LINE);
}

int CmdHandler(int fd, Message *msg)
{
    int ret = ReadSizeN(fd, msg->msg, msg->msg_len);
    if (ret != msg->msg_len) {
        fprintf(stderr, "CmdHandler read error:%d\n", errno);
        return -1;
    }
    printf("received client msg: %s, len: %d\n", msg->msg, strlen(msg->msg));

    // 回写消息
    Message ret_msg;
    ret_msg.msg_type = htonl(MSG_TYPE_CONTENT);
    if (msg->msg_len == 2) {
        if (strncmp(msg->msg, "ls", 2) == 0) {
            CmdLs(ret_msg.msg);
        } else {
            CmdUnknow(ret_msg.msg, msg->msg);
        }
    } else if (msg->msg_len == 3) {
        if (strncmp(msg->msg, "pwd", 3) == 0) {
            CmdPwd(ret_msg.msg);
        } else {
            CmdUnknow(ret_msg.msg, msg->msg);
        }
    } else {
        if (strncmp(msg->msg, "cd", 2) == 0) {
            CmdCd(ret_msg.msg, msg->msg);
        } else {
            CmdUnknow(ret_msg.msg, msg->msg);
        }
    }
    ret_msg.msg_len = htonl(strlen(ret_msg.msg));
    int send_len = sizeof(ret_msg.msg_len) + sizeof(ret_msg.msg_type) + strlen(ret_msg.msg);
    if (send(fd, (char *)&ret_msg, send_len, 0) < 0) {
        fprintf(stderr, "CmdHandler send to client error:%d\n", errno);
        return -1;
    }

    return 0;
}

int ClientHandler(int fd)
{
    Message msg;
    // 读取消息长度
    int ret = read(fd, &msg.msg_len, sizeof(msg.msg_len));
    if (ret != sizeof(msg.msg_len)) {
        fprintf(stderr, "ClientHandler read msg len[%d] error\n", ret);
        return -1;
    }
    msg.msg_len = ntohl(msg.msg_len);
    // 读取类型
    ret = read(fd, &msg.msg_type, sizeof(msg.msg_type));
    if (ret != sizeof(msg.msg_type)) {
        fprintf(stderr, "ClientHandler read msg type len[%d] error\n", ret);
        return -1;
    }
    msg.msg_type = ntohl(msg.msg_type);

    switch (msg.msg_type) {
        case MSG_TYPE_PING:
            printf("received ping msg.\n");
            return SendPing(fd);
        case MSG_TYPE_CONTENT:
            return CmdHandler(fd, &msg);
        default :
            fprintf(stderr, "unknow client msg type[%d]\n", msg.msg_type);
            return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int listen_fd = TcpServer(argv[1]);

    fd_set init_read_set;
    fd_set read_set;
    FD_ZERO(&init_read_set);
    FD_SET(listen_fd, &init_read_set);
    int MAX_FD = 1024;

    char old_dir[MAX_LINE];
    if (getcwd(old_dir, MAX_LINE) == NULL) {
        ErrorExit("getcwd");
    }

    while (1) {
        read_set = init_read_set;
        int ret = select(MAX_FD, &read_set, NULL, NULL, NULL);
        if (ret < 0) {
            ErrorExit("select");
        }
        for (int i = 0; i < MAX_FD; ++i) {
            if (FD_ISSET(i, &read_set)) {
                if (i == listen_fd) {
                    AcceptNew(listen_fd, &init_read_set);
                } else {
                    ret = ClientHandler(i);
                    if (ret < 0) {
                        FD_CLR(i, &init_read_set);
                        chdir(old_dir);
                    }
                }
            }
        }
    }

    return 0;
}