/* xtime -- extended-resolution integer time stamps */

#ifndef XTIME_H
#define XTIME_H

/* xtime_t is a signed type used for time stamps. It is an integer
   type that is a count of nanoseconds -- except for obsolescent hosts
   without sufficiently-wide integers, where it is a count of
   seconds */
typedef long long int xtime_t;
#define XTIME_PRECISION 1000000000

/* Return an extended time value that contains S seconds and NS
   nanoseconds, without any overflow checking */
static inline xtime_t
xtime_make(xtime_t s, long int ns)
{
    if(XTIME_PRECISION == 1)
        return s;
    else
        return XTIME_PRECISION * s + ns;
}


#endif
