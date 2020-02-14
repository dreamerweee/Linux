#include "memory_pool.h"

int main()
{
    size_t alloc_len = sizeof(int) * 10;
    int *parr = (int*)Allocate(alloc_len);
    for (int i = 0; i < 10; ++i) {
        parr[i] = i;
    }

    for (int i = 9; i >= 0; --i) {
        printf("%d ", parr[i]);
    }
    printf("\n");

    Deallocate(parr, alloc_len);
    return 0;
}