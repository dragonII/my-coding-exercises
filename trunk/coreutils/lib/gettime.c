/* gettime -- get the system clock */

/* Get the system time into *TS */

#include "gettime.h"

void gettime(struct timespec* ts)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
}
