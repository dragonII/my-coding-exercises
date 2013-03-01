/* gethrxtime -- get high resolution real time */

#ifndef GETHRXTIME_H
#define GETHRXTIME_H

#include "xtime.h"

/* Get the current time, as a count of the number of nanoseconds since
   an arbitrary epoch (e.g., the system boot time). Prefer a
   high-resolution clock that is not subject to resetting or
   drifting */
xtime_t gethrxtime(void);


#endif
