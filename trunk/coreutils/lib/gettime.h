/* gettime -- get the system clock */

#ifndef GETTIME_H
#define GETTIME_H

#include <time.h>
#include <sys/time.h>

void gettime(struct timespec*);


#endif
