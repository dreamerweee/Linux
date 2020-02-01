#ifndef SOCKET_PACK_H
#define SOCKET_PACK_H

/*
* 该头文件定义各个原始socket接口的包裹函数，出错时直接终止进程
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE_BUFF 1024
#define SERV_PORT 5678  // 服务器监听端口
#define MAX_EVENT_NUMBER 1024  // 事件最大值

void ErrSys(const char *fmt, ...);
void ErrExit(const char *fmt, ...);
void InetPton(int family, const char *str_ptr, void *addr_ptr);
void InetNtop(int family, const void *addr_ptr, char *str_ptr, size_t len);
int Socket(int domain, int type, int protocol);
void Bind(int sock_fd, const struct sockaddr *addr, socklen_t addr_len);
void Listen(int sock_fd, int back_log);
int Accept(int sock_fd, struct sockaddr *addr, socklen_t *addr_len);
void Close(int fd);
void Connect(int sock_fd, const struct sockaddr *serv_addr, socklen_t addr_len);
void Shutdown(int sock_fd, int howto);
int SetNonblocking(int fd);

#endif  //SOCKET_PACK_H