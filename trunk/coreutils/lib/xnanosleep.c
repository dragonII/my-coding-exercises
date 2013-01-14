/* xnanosleep.c -- a more convenient interface to nanosleep */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

#include "xnanosleep.h"
#include "intprops.h"

#ifndef TIME_T_MAX
# define TIME_T_MAX TYPE_MAXIMUM(time_t)
#endif

/* Sleep until the time (call it WAKE_UP_TIME) specified as
   SECONDS seconds after the time this function is called.
   SECONDS must be non-negative. If SECONDS is so large that
   it is not representable as a `struct timespec', then use
   the maximum value for that interval. Return -1 on failure
   (setting errno), 0 on success. */
int xnanosleep(double seconds)
{
    enum { BILLION = 1000000000 };

    /* For overflow checking, use naive comparison if possible, widening
       to long double if double is not wide enough. Otherwise, use <=,
       not <, to avoid problems when TIME_T_MAX is less than SECONDS but
       compares equal to SECONDS after loss of precision when coercing
       from time_t to long double. This mishandles near-maximal values
       in some rare (perhaps theoretical) cases but that is better than
       undefined behaviour. */

    bool overflow = ((time_t)((double)TIME_T_MAX / 2) == TIME_T_MAX / 2
                    ? TIME_T_MAX < seconds
                    : (time_t)((long double)TIME_T_MAX / 2) == TIME_T_MAX / 2
                        ? TIME_T_MAX < (long double)seconds
                        : TIME_T_MAX <= (long double)seconds);

    struct timespec ts_sleep;

    assert(seconds >= 0);

    /* Separate whole seconds from nanoseconds. */
    if(!overflow)
    {
        time_t floor_seconds = seconds;
        double ns = BILLION * (seconds - floor_seconds);
        ts_sleep.tv_sec = floor_seconds;

        /* Round up to the next whole number, if necessary, so that we
           always sleep for at least the requested amount of time. Assuming
           the default rounding mode, we don't have to worry about the 
           rounding error when computing 'ns' above, since the error won't
           cause 'ns' to drop below an integer boundary */
        ts_sleep.tv_nsec = ns;
        ts_sleep.tv_nsec += (ts_sleep.tv_nsec < ns);

        /* Normalize the interval length. nanosleep requires this */
        if(BILLION <= ts_sleep.tv_nsec)
        {
            if(ts_sleep.tv_sec == TIME_T_MAX)
                overflow = true;
            else
            {
                ts_sleep.tv_sec++;
                ts_sleep.tv_nsec -= BILLION;
            }
        }
    }

    for(;;)
    {
        if(overflow)
        {
            ts_sleep.tv_sec = TIME_T_MAX;
            ts_sleep.tv_nsec = BILLION - 1;
        }

        /* Linux-2.6.8.1's nanosleep return -1, but doesn't set errno
           when resumed after being suspended. Earlier versions would
           set errno to EINTR. nanosleep from linux-2.6.10, as well as
           implementations by (all?) other vendors, doesn't return -1
           in that case; either it continues sleeping (if time remains)
           or it return zero (if the wake-up time has passed) */
        errno = 0;
        if(nanosleep(&ts_sleep, NULL) == 0)
            break;
        if(errno != EINTR && errno != 0)
            return -1;
    }

    return 0;
}
