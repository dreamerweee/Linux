#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#define MAXLINE 4096

typedef struct sockaddr SA;

static void ErrOutput(const char *format, va_list va)
{
	char buf[MAXLINE + 1];
	vsnprintf(buf, MAXLINE, format, va);
	int len = strlen(buf);
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
	ErrOutput(format, va);
	va_end(va);
	exit(1);
}

#endif //UTIL_H