#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

static const int kBufferSize = 1024;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: copy_file src_file_name dst_file_name\n");
        exit(1);
    }

    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0) {
        perror("open");
        exit(1);
    }
    int dst_fd = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
    if (dst_fd < 0) {
        perror("open");
        exit(1);
    }

    char buff[kBufferSize];
    ssize_t len = 0;
    while ((len = read(src_fd, buff, kBufferSize)) > 0) {
        if (write(dst_fd, buff, len) != len) {
            perror("write failed");
            break;
        }
    }
    if (len < 0) {
        perror("read failed");
        exit(1);
    }

    close(src_fd);
    close(dst_fd);

    exit(0);
}