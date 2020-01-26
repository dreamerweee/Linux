#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s [dir]\n", argv[0]);
        return -1;
    }
    if (chdir(argv[1]) == -1) {
        printf("chdir error: %s\n", strerror(errno));
        return -1;
    }
    printf("current pwd: %s\n", getcwd(NULL, 0));
    return 0;
}