#include "common.h"

static int Common(size_t size, int flags)
{
    key_t key = ftok(PATHNAME, PROJ_ID);
    if (key < 0) {
        printf("ftok failed.\n");
        exit(1);
    }
    int shm_id = shmget(key, size, flags);
    if (shm_id < 0) {
        printf("shmget failed.\n");
        exit(1);
    }
    return shm_id;
}

int CreateShm(size_t size)
{
    return Common(size, IPC_CREAT | IPC_EXCL | 0666);
}

int GetShm()
{
    return Common(0, IPC_CREAT);
}

int DestroyShm(int shm_id)
{
    if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
        printf("shmctl failed.\n");
        return -1;
    }
    return 0;
}
