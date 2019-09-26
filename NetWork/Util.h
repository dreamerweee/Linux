#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 4096

typedef struct sockaddr SA;

static void ErrOutput(int errno_flag, const char *format, va_list va)
{
	int errno_old = errno;
	char buf[MAXLINE + 1];
	vsnprintf(buf, MAXLINE, format, va);
	int len = strlen(buf);
	if (errno_flag) {
		snprintf(buf + len, MAXLINE - len, ": %s", strerror(errno_old));
	}
	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf, stderr);
	fflush(stderr);

	return;
}

void ErrQuit(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	ErrOutput(0, format, va);
	va_end(va);
	exit(1);
}

// 查看errno变量的值并输出相应的出错消息
void ErrSys(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	ErrOutput(1, format, va);
	va_end(va);
	exit(1);
}

#endif //UTIL_H