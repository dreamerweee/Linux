#ifndef __common_h__
#define __common_h__

#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>

double GetTime()
{
  struct timeval t;
  int res = gettimeofday(&t, NULL);
  assert(res == 0);
  return (double)t.tv_sec + (double)t.tv_usec/1e6;
}

void Spin(int how_long)
{
  double t = GetTime();
  while ((GetTime() - t) < (double)how_long)
    ;
}

#endif // __common_h__