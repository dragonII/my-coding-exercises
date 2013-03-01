/* gethrxtime -- get high resolution real time */

#include "gethrxtime.h"
#include "gettime.h"

#include <sys/time.h>

xtime_t gethrxtime(void)
{
    struct timespec ts;
    gettime(&ts);
    return xtime_make(ts.tv_sec, ts.tv_nsec);
}
