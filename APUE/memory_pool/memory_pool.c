#include "memory_pool.h"


// 已分配内存且未被管理的内存
char *allocated_begin = 0;
char *allocated_end = 0;

// 内存对象
typedef struct MemObj {
    struct MemObj *next;
} MemObj;

// 已分配且被数组管理的内存
// [0] = 8bytes, [1] = 16bytes, [2] = 24bytes,...[15] = 128bytes
MemObj *allocated_list[16] = {0};

const size_t kAlign = 8;
const size_t kMaxBytes = 128;  // 申请内存大于此值，直接走Malloc

// 字节对齐函数
static size_t BytesRoundUp(size_t bytes)
{
    return ((bytes + kAlign - 1) & ~(kAlign - 1));
}

// 获取字节对应的内存数组下标
static size_t GetListIndex(size_t bytes)
{
    return (bytes + kAlign - 1) / kAlign - 1;
}

// 封装的malloc
static void* Malloc(size_t nbytes)
{
    void *result = malloc(nbytes);
    if (!result) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return result;
}

// free接口
static void Free(void *ptr)
{
    free(ptr);
}

// 分配一大块内存
// pcount 为输出型参数，表示最终分配了*pcount 个 bytes 大小的内存
static char* AllocateChunk(size_t bytes, int *pcount)
{
    char *result;
    size_t total_bytes = bytes * (*pcount);
    size_t left_bytes = allocated_end - allocated_begin;  // 内存池中剩余的字节数量
    if (left_bytes >= total_bytes) {
        result = allocated_begin;
        allocated_begin += total_bytes;
        return result;
    } else if (left_bytes >= bytes) {  // 至少满足申请的1个bytes大小的内存
        *pcount = left_bytes / bytes;
        result = allocated_begin;
        allocated_begin += bytes * (*pcount);
        return result;
    } else {  // 从新分配一块内存
        // 充分利用剩下的字节数，把剩下的内存放入内存数组
        if (left_bytes > 0) {
            MemObj **list_begin = allocated_list + GetListIndex(left_bytes);
            ((MemObj*)allocated_begin)->next = *list_begin;
            *list_begin = (MemObj*)allocated_begin;
        }
        // 分配2倍 total_bytes 大的内存
        size_t malloc_bytes = 2 * total_bytes;
        allocated_begin = (char*)malloc(malloc_bytes);
        if (!allocated_begin) {  // 分配失败
            // 从比它大的内存数组去找
            MemObj **list_begin;
            MemObj *pobj;
            for (size_t b = bytes; b <= kMaxBytes; b += kAlign) {
                list_begin = allocated_list + GetListIndex(b);
                pobj = *list_begin;
                if (pobj) {  // 内存数组存在>=bytes的内存块，直接分配
                    *list_begin = pobj->next;
                    allocated_begin = (char*)pobj;
                    allocated_end = allocated_begin + b;
                    return AllocateChunk(bytes, pcount);
                }
            }
            // 到这里说明没有内存，调用 Malloc
            // allocated_end = 0;
            allocated_begin = (char*)Malloc(malloc_bytes); // 这里要么成功，要么异常退出
        }
        // 分配成功
        allocated_end = allocated_begin + malloc_bytes;
        return AllocateChunk(bytes, pcount);
    }
}

// 从分配的内存中分配一块出来给内存数组管理
// bytes 是已经按照 kAlign 对齐了的字节数
static void* AllocateDistribute(size_t bytes)
{
    int count = 20;  // 一次性分配20个 bytes
    char *chunk = AllocateChunk(bytes, &count);
    if (count == 1) {
        return chunk;
    }
    // 有多个bytes，第一个返回给申请者，剩余的放入内存数组
    void *result = (void*)chunk;
    MemObj **list_begin = allocated_list + GetListIndex(bytes);
    *list_begin = (MemObj*)(chunk + bytes);  // *list_begin肯定是没有的，所以可以直接赋值
    MemObj *current_obj;
    MemObj *next_obj = *list_begin;
    for (int i = 1; ; ++i) {
        current_obj = next_obj;
        next_obj = (MemObj*)((char*)next_obj + bytes);
        if (i + 1 == count) {
            current_obj->next = 0;
            break;
        } else {
            current_obj->next = next_obj;
        }
    }

    return result;
}

void* Allocate(size_t nbytes)
{
    if (nbytes == 0) {
        return NULL;
    }
    void *result = NULL;
    // 申请内存大于 kMaxBytes 时，直接走Malloc分配内存
    if (nbytes > kMaxBytes) {
        result = Malloc(nbytes);
    } else {  // 先查看内存数组中是否有剩余空间
        // 查找数组下标时，做了一个对齐提升，就是将申请的内存提升到 kAlign 的整数倍
        MemObj **list_begin = allocated_list + GetListIndex(nbytes);
        MemObj *lst_obj = *list_begin;
        if (!lst_obj) {  // 该空间已经没有了，去内存池找
            result = AllocateDistribute(BytesRoundUp(nbytes));
        } else {
            *list_begin = lst_obj->next;
            result = lst_obj;
        }
    }
    return result;
}

void Deallocate(void *ptr, size_t nbytes)
{
    if (nbytes == 0) {
        return;
    }
    if (nbytes > kMaxBytes) {
        Free(ptr);
    } else {  // 重新放入内存数组
        MemObj **list_begin = allocated_list + GetListIndex(nbytes);
        MemObj *list_obj = (MemObj*)ptr;
        list_obj->next = *list_begin;
        *list_begin = list_obj;
    }
}
