#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// od -c test.hole  查看文件实际内容

int main()
{
	mode_t mode = S_IRUSR |  S_IWUSR;
	int fd = open("test.hole", O_RDWR | O_CREAT | O_TRUNC, mode);
	if (fd < 0) {
		perror("open file[test.hole] failed:\n");
		exit(-1);
	}

	int ret = write(fd, "hello", 5);
	if (ret != 5) {
		perror("write file error:\n");
		exit(-1);
	}

	off_t off = 20;
	ret = lseek(fd, off, SEEK_CUR);
	if (ret < 0) {
		perror("lseek error:\n");
		exit(-1);
	}

	ret = write(fd, "world", 5);
	if (ret != 5) {
		perror("write error:\n");
		exit(-1);
	}

	lseek(fd, 0, SEEK_SET);
	char buff[100];
	ret = read(fd, buff, 100);
	if (ret < 0) {
		perror("read error:\n");
	} else {
		printf("read content: %s\n", buff);
	}
	return 0;
}