#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/types.h>

#define PATHNAME  "."
#define PROJ_ID   0x6666

int CreateShm(size_t size);
int GetShm();
int DestroyShm(int shm_id);

#endif  // COMMON_H