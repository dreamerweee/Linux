#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int g_value = 0;
char g_buffer[] = "need write message\n";

int main()
{
	int value = 10;
	int msg_len = strlen(g_buffer);
	if (write(STDOUT_FILENO, g_buffer, msg_len) != msg_len) {
		printf("write error\n");
		exit(-1);
	}
	printf("before fork\n");  // 终端设备为行缓存，而文件是全缓存
	pid_t pid = fork();
	if (pid < 0) {
		printf("fork error\n");
		exit(-1);
	} else if (pid == 0) {
		++g_value;
		++value;
	} else {
		sleep(2);
	}

	printf("pid = %ld, global value = %d, local value = %d\n", getpid(), g_value, value);
	exit(0);
}