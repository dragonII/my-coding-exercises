#include "file_.h"
#include "cdf.h"

#include <errno.h>


#define isleap(y)   ((( (y) % 4) == 0) && ((( (y) % 100) != 0) || (( (y) % 400) == 0)))

static const int mdays[] = 
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* Return the number of days between jan 01 1601 and jan 01 of year */
static int cdf_getdays(int year)
{
    int days = 0;
    int y;

    for(y = CDF_BASE_YEAR; y < year; y++)
        days += isleap(y) + 365;

    return days;
}

/* Return the day within the month */
static int cdf_getday(int year, int days)
{
    size_t m;

    for(m = 0; m < sizeof(mdays) / sizeof(mdays[0]); m++)
    {
        int sub = mdays[m] + (m == 1 && isleap(year));
        if(days < sub)
            return days;
        days -= sub;
    }
    return days;
}

/* Return the 0 ... 11 month number */
static int cdf_getmonth(int year, int days)
{
    size_t m;

    for(m = 0; m < sizeof(mdays) / sizeof(mdays[0]); m++)
    {
        days -= mdays[m];
        if(m == 1 && isleap(year))
            days--;
        if(days <= 0)
            return (int)m;
    }

    return (int)m;
}

int cdf_timestamp_to_timespec(struct timespec* ts, cdf_timestamp_t t)
{
    struct tm tm;
    static char UTC[] = "UTC";

    int rdays;

    /* Unit is 100's of nanoseconds */
    ts->tv_nsec = (t % CDF_TIME_PREC) * 100;

    t /= CDF_TIME_PREC;
    tm.tm_sec = (int)(t % 60);
    t /= 60;

    tm.tm_min = (int)(t % 60);
    t /= 60;

    tm.tm_hour = (int)(t % 24);
    t /= 24;

    tm.tm_year = (int)(CDF_BASE_YEAR + (t / 365));

    rdays = cdf_getdays(tm.tm_year);
    t -= rdays - 1;
    tm.tm_mday = cdf_getday(tm.tm_year, (int)t);
    tm.tm_mon = cdf_getmonth(tm.tm_year, (int)t);
    tm.tm_wday = 0;
    tm.tm_yday = 0;
    tm.tm_isdst = 0;
    tm.tm_zone = UTC;
    tm.tm_year -= 1900;
    ts->tv_sec = mktime(&tm);
    if(ts->tv_sec == -1)
    {
        errno = EINVAL;
        return -1;
    }
    return 0;
}
