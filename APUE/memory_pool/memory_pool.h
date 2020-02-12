#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stdio.h>
#include <stdlib.h>

/*
* 内存池的简单实现
*/

// 申请内存
void* Allocate(size_t nbytes);

// 释放内存
void Deallocate(void *ptr, size_t nbytes);

#endif // MEMORY_POOL_H