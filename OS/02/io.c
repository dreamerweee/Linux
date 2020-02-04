#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    // S_IRWXU = S_IRUSR | S_IWUSR | S_IXUSR
    int fd = open("/tmp/file", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    assert(fd >= 0);
    int write_len = write(fd, "hello world\n", 13);
    assert(write_len == 13);
    close(fd);
    return 0;
}