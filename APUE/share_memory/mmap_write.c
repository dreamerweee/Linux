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
    int fd = open(argv[1], O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        printf("open %s failed.\n", argv[1]);
        exit(-1);
    }
    if (ftruncate(fd, sizeof(Message)) == -1) {
        printf("ftruncate failed.\n");
        exit(-1);
    }
    Message *p = (Message*)mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        printf("mmap failed[%s].\n", strerror(errno));
        exit(-1);
    }
    close(fd);   // 映射完成后可以关闭文件
    Message msg = {1, "weee", 1};
    while (msg.age < 200) {
        memcpy(p, &msg, sizeof(msg));
        ++msg.age;
        ++msg.idx;
        printf("write message: %d %s %d\n", msg.idx, msg.name, msg.age);
        sleep(2);
    }

    if (munmap(p, sizeof(msg)) < 0) {
        printf("munmap failed.\n");
        exit(-1);
    }

    return 0;
}
