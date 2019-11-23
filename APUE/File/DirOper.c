#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void ReadDir(const char *name, int level)
{
	DIR *p_dir = opendir(name);
	if (!p_dir) {
		printf("opendir error[%s]\n", strerror(errno));
		exit(-1);
	}

	chdir(name);

	struct dirent *p_dirent;
	while ((p_dirent = readdir(p_dir)) != NULL) {
		if (strcmp(p_dirent->d_name, "..") == 0 || strcmp(p_dirent->d_name, ".") == 0) {
			continue;
		}

		int curr_lvl = level;
		while (curr_lvl > 0) {
			printf("\t");
			--curr_lvl;
		}
		printf("name: %s\n", p_dirent->d_name);

		struct stat stat_buf;
		int ret = stat(p_dirent->d_name, &stat_buf);
		if (ret != -1) {
			if (S_ISDIR(stat_buf.st_mode)) {
				ReadDir(p_dirent->d_name, level + 1);
				chdir("../");
			}
		} else {
			printf("stat error[%s]\n", strerror(errno));
		}
	}
	closedir(p_dir);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: %s [dirname]\n", argv[0]);
		exit(-1);
	}
	ReadDir(argv[1], 0);

	return 0;
}