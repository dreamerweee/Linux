#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    time_t now_time = time(NULL);
    struct tm *ptr_gm = gmtime(&now_time);
    struct tm *ptr_local = localtime(&now_time);
    printf("now time = %ld\n", now_time);
    printf("gm year: %d %d\n", 1900 + ptr_gm->tm_year, ptr_gm->tm_mday);
    printf("local year: %d %d\n", 1900 + ptr_local->tm_year, ptr_local->tm_mday);

    time_t t = mktime(ptr_gm);
    printf("t = %ld\n", t);
    return 0;
}