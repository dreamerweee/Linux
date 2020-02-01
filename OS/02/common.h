#ifndef COMMON_H
#define COMMON_H

#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>

double GetTime()
{
    struct timeval tv;
    int ret = gettimeofday(&tv, NULL);
    assert(ret == 0);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

void Spin(int how_long)
{
    double time = GetTime();
    while ((GetTime() - time) < (double)how_long)
        ;  // do nothing in loop
}

#endif // COMMON_H