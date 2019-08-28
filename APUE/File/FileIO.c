#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static int OpenFile(const char *name)
{
	// mod受umask环境变量的影响，系统默认情况下，umask的值为八进制0022，表示组用户和其他用户的写权限被取消了
	mode_t mod = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
	int fd = open(name, O_RDWR | O_CREAT, mod);
	if (fd == -1) {
		printf("OpenFile: failed error[%s]\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return fd;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("FileIO: usage: %s openfilename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd = OpenFile(argv[1]);

	const char buff[] = "hello world!";
	ssize_t len = write(fd, buff, strlen(buff));
	if (len < 0) {
		printf("FileIO: write: %s\n", strerror(errno));
	} else {
		printf("FileIO: write: len[%ld]\n", len);
	}

	// write 完成后，当前文件的偏移量在结尾处，所以重新设置偏移量以便读取数据
	lseek(fd, 1, SEEK_SET);

	char rd_buff[10];
	len = read(fd, rd_buff, 10);
	if (len < 0) {
		printf("FileIO: read: %s\n", strerror(errno));
	} else {
		printf("FileIO: read: len[%ld] msg[%s]\n", len, rd_buff);
	}

	close(fd);
	exit(EXIT_SUCCESS);
}