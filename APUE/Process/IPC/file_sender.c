#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s [pid]\n", argv[0]);
        return -1;
    }
    
    pid_t recv_pid = atoi(argv[1]);

    int fd = open("tmp", O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        perror("open:");
        return -1;
    }
    char buff[1024];
    int read_len;
    while ((read_len = read(STDIN_FILENO, buff, 1024)) > 0) {
        if (write(fd, buff, read_len) != read_len) {
            perror("write:");
            break;
        }
    }
    close(fd);
    if (kill(recv_pid, SIGUSR1) < 0 ) {
        perror("kill:");
        return -1;
    }
    return 0;
}