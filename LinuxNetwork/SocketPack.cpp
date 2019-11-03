#include "SocketPack.h"

static void ErrPrint(int errno_flag, const char *fmt, va_list args)
{
	int errno_save = errno;
	char msg[MAX_LINE_BUFF + 1];

	vsnprintf(msg, MAX_LINE_BUFF, fmt, args);

	if (errno_flag) {
		int len = strlen(msg);
		snprintf(msg + len, MAX_LINE_BUFF - len, ": %s", strerror(errno_save));
	}
	strcat(msg, "\n");

	fflush(stdout);
	fputs(msg, stderr);
	fflush(stderr);
}

// 因系统错误退出，需要输出errno信息
void ErrSys(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ErrPrint(1, fmt, args);
	va_end(args);
	exit(1);
}

void ErrExit(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ErrPrint(0, fmt, args);
	va_end(args);
	exit(1);
}

void InetPton(int family, const char *str_ptr, void *addr_ptr)
{
	int ret = inet_pton(family, str_ptr, addr_ptr);
	if (ret == 0) {
		ErrExit("inet_pton str_ptr[%s] error", str_ptr);
	} else if (ret < 0) {
		ErrSys("inet_pton error");
	}
}

void InetNtop(int family, const void *addr_ptr, char *str_ptr, size_t len)
{
	if (inet_ntop(family, addr_ptr, str_ptr, len) == NULL) {
		ErrSys("inet_ntop error");
	}
}

int Socket(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd < 0) {
		ErrSys("socket error");
	}
	return fd;
}

void Bind(int sock_fd, const struct sockaddr *addr, socklen_t addr_len)
{
	if (bind(sock_fd, addr, addr_len) < 0) {
		ErrSys("bind error");
	}
}

void Listen(int sock_fd, int back_log)
{
	if (listen(sock_fd, back_log) < 0) {
		ErrSys("listen error");
	}
}

int Accept(int sock_fd, struct sockaddr *addr, socklen_t *addr_len)
{
	int conn_fd = accept(sock_fd, addr, addr_len);
	if (conn_fd < 0) {
		ErrSys("accept error");
	}
	return conn_fd;
}

void Close(int fd)
{
	if (close(fd) < 0) {
		ErrSys("close error");
	}
}

void Connect(int sock_fd, const struct sockaddr *serv_addr, socklen_t addr_len)
{
	if (connect(sock_fd, serv_addr, addr_len) < 0) {
		ErrSys("connect error");
	}
}

void Shutdown(int sock_fd, int howto)
{
	if (shutdown(sock_fd, howto) < 0) {
		ErrSys("shutdown error");
	}
}

int SetNonblocking(int fd)
{
	int old_opt = fcntl(fd, F_GETFL);
	if (fcntl(fd, F_SETFL, old_opt | O_NONBLOCK) < 0) {
		ErrSys("fcntl error");
	}
	return old_opt;
}
