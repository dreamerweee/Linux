#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>

typedef struct {
    int idx;
    char name[20];
    int age;
} Message;

int main(int argc, char *argv[])
{
    assert(argc == 2);
    
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("open failed.\n");
        exit(-1);
    }
    Message *p = (Message *)mmap(NULL, sizeof(Message), PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        printf("mmap failed[%s].\n", strerror(errno));
        exit(-1);
    }
    close(fd);
    while (p->age < 200) {
        printf("the shared message: %d %s %d\n", p->idx, p->name, p->age);
        sleep(2);
    }
    if (munmap(p, sizeof(Message)) < 0) {
        printf("munmap failed.\n");
        exit(-1);
    }

    return 0;
}