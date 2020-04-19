#include "common.h"

int main(void)
{
    int shm_id = GetShm();
    printf("shm_id = %d\n", shm_id);

    char *buff = (char *)shmat(shm_id, NULL, 0);
    while (1) {
        printf("%s\n", buff);
        sleep(5);
    }
    shmdt(buff);
    return 0;
}