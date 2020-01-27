#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main()
{
    int fd = open("test.txt", O_CREAT | O_WRONLY, 0664);
    if (fd == -1) {
        perror("open file:");
        return -1;
    }

    printf("before fork:\n");
    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork:");
        return -1;
    } else if (pid > 0) {  // father
        sleep(3);
        printf("in father process\n");
        write(fd, " hhh ", 5);

        pid_t w_pid = wait(NULL);
        printf("end process pid[%lld] w_pid[%lld]\n", getpid(), w_pid);
    } else {
        printf("in child process\n");
        write(fd, "hello ", 6);
        sleep(5);
        write(fd, "world", 5);
    }
    printf("process pid[%lld] end\n", getpid());
    
    close(fd);
    return 0;
}