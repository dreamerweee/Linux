#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define MAXLINE 1024

int main(int argc, char *argv[])
{
	pid_t pid;
	char cmd_buff[MAXLINE];
	while (fgets(cmd_buff, MAXLINE, stdin) != NULL) {
		if (cmd_buff[strlen(cmd_buff) - 1] == '\n') {
			cmd_buff[strlen(cmd_buff) - 1] = '\0';
		}
		if ((pid = fork()) < 0) {
			printf("fork error[%s]\n", strerror(errno));
			exit(-1);
		} else if (pid == 0) {  // child
			execlp(cmd_buff, cmd_buff, (char *)0);
			printf("execlp error.\n");
			exit(-1);
		}

		int status;
		if ((pid = waitpid(pid, &status, 0)) < 0) {
			printf("waitpid error.\n");
		}
	}
	perror("not find error");

	return 0;
}