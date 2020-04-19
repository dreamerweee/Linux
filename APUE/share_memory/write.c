#include "common.h"

int main(void)
{
    int shm_id = CreateShm(4096);
    printf("shm_id = %d\n", shm_id);

    char *buff = shmat(shm_id, NULL, 0);
    printf("buff = %p\n", buff);
    int i = 0;
    while (i < 20) {
        buff[i] = 'A' + i % 26;
        ++i;
        buff[i] = 0;
        sleep(5);
    }
    shmdt(buff);
    DestroyShm(shm_id);

    return 0;
}