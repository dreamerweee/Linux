#include "../SocketPack.h"
#include <signal.h>

void SigPipe(int signo)
{
	printf("get signal: %d\n", signo);
	exit(signo);
}

int main()
{
	signal(SIGPIPE, SigPipe);

	int fd[2];
	if (pipe(fd) < 0) {
		printf("pipe error: %s\n", strerror(errno));
		exit(-1);
	}

	close(fd[0]);
	const char *msg = "hello";
	int ret = write(fd[1], msg, strlen(msg));
	if (ret < 0) {
		printf("write error: %s\n", strerror(errno));
		exit(-1);
	}
	exit(0);
}
