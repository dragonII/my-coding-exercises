#include <ptrx_times.h>
#include <ptrx_string.h>
#include <ptrx_atomic.h>


/*
 * The time may be updated by signal handler or by several threads.
 * The time update operations are rare and require to hold the
 * ptrx_time_lock.
 * The time read operations are frequent, so they are lock-free and 
 * get time values and strings from the current slot. Thus thread may
 * get the corrupted values only if it is preempted while copying
 * and then it is not scheduled to run more than PTRX_TIME_SLOT seconds.
 */

#define PTRX_TIME_SLOT      64

static unsigned int     slot;
static ptrx_atomic_t    ptrx_time_lock;

volatile ptrx_msec_t    ptrx_current_msec;
volatile ptrx_time_t   *ptrx_cached_time;
volatile ptrx_str_t     ptrx_cached_err_log_time;
volatile ptrx_str_t     ptrx_cached_http_time;
volatile ptrx_str_t     ptrx_cached_http_log_iso8601;
volatile ptrx_str_t     ptrx_cached_http_time;


/*
 * localtime() and localtime_r() are not Async-Signal-Safe function, 
 * therefore, they must not be called by a signal handler, so we use
 * the cached GMT offset value. Fortunately the value is changed only 
 * two times a year.
 */
static int  cached_gmtoff;

static ptrx_time_t      cached_time[PTRX_TIME_SLOT];

void ptrx_gmtime(time_t t, ptrx_tm_t *tp)
{
    int             yday;
    unsigned int    n, sec, min, hour, mday, mon, year, wday, days, leap;

    /* the calculation is valid for positive time_t only */

    n = (unsigned int)t;

    days = n / 86400; // 1 day = 86400 s

    /* January 1, 1970 was Thursday */
    wday = (4 + days) % 7;

    n %= 86400;
    hour = n / 3600;
    n %= 3600;
    min = n / 60;
    sec = n % 60;

    /* 
     * The algorithm based on Gauss' formula,
     * TODO: see src/http/ptrx_http_parse_time.c
     */

    /* days since March 1, 1 BC */
    days = days - (31 + 28) + 719527;

    /* The "days" should be adjusted to 1 only, however, some March 1st's
     * go to previous year, so we adjust them to 2. This causes also 
     * shift of the last Feburary days to next year, but we catch the 
     * case when "yday" becomes negative.
     */
    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);
    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

    if(yday < 0)
    {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0))
        yday = 365 + leap + yday;
        year--;
    }

    /* 
     * The empirical formular that maps "yday" to month.
     * There are at least 10 variants, some of them are:
     *      mon = (yday + 31) * 15 / 459
     *      mon = (yday + 31) * 17 / 520
     *      mon = (yday + 31) * 20 / 612
     */

    mon = (yday + 31) * 10 / 306;

    /* The Gauss' formula that evaluates days before the month */

    mday = yday - (367 * mon / 12 - 30) + 1;

    if(yday >= 306)
    {
        year++;
        mon -= 10;
    } else
    {
        mon += 2;
    }

    tp->tm_sec = (ngx_tm_sec_t) sec;
    tp->tm_min = (ngx_tm_min_t) min;
    tp->tm_hour = (ngx_tm_hour_t) hour;
    tp->tm_mday = (ngx_tm_mday_t) mday;
    tp->tm_mon = (ngx_tm_mon_t) mon;
    tp->tm_year = (ngx_tm_year_t) year;
    tp->tm_wday = (ngx_tm_wday_t) wday;
}



void ptrx_time_init(void)
{
    ptrx_cached_err_log_time.len = sizeof("1970/09/28 12:00:00") - 1;
    ptrx_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
    ptrx_cached_log_time.len = sizeof("28/Sep/1970:12:00:00 +0600") - 1;
    ptrx_cached_http_log_iso8601.len = sizeof("1970-09-28T12:00:00+06:00") - 1;

    ptrx_cached_time = &cached_time[0];

    ptrx_time_update();
}


void ptrx_localtime(time_t s, ptrx_tm_t *tm)
{
    ptrx_tm_t *t;

    t = localtime(&s);
    *tm = *t;

    tm->tm_mon++;
    tm->tm_year += 1900;
}


void ptrx_time_update(void)
{
    unsigned char       *p0, *p1, *p2, *p3;
    ptrx_tm_t           tm, gmt;
    time_t              sec;
    unsigned int        msec;
    ptrx_time_t         *tp;
    struct timeval      tv;

    if(!ptrx_trylock(&ptrx_time_lock))
    {
        return;
    }

    ptrx_gettimeofday(&tv);

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;

    ptrx_current_msec = (ptrx_msec_t) sec * 1000 + msec;

    tp = &cached_time[slot];

    if(tp->sec == sec)
    {
        tp->msec = msec;
        ptrx_unlock(&ptrx_time_lock);
        return;
    }

    if(slot == PTRX_TIMES_SLOTS - 1)
    {
        slot = 0;
    } else
    {
        slot++;
    }

    tp = &cached_time[slot];

    tp->sec = sec;
    tp->msec = msec;

    ptrx_gmtime(sec, &gmt);

    p0 = &cached_http_time[slot][0];

    (void)ptrx_sprintf(p0, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                       week[gmt.tm_wday], gmt.tm_mday,
                       months[gmt.tm_mon - 1], gmt.tm_year,
                       gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

    ptrx_localtime(sec, &tm);

    p1 = &cached_err_log_time[slot][0];

    (void)ptrx_sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
                       tm.tm_year, tm.tm_mon,
                       tm.tm_mday, tm.tm_hour,
                       tm.tm_min, tm.tm_sec);

    p2 = &cached_http_log_time[slot][0];

    (void)ptrx_sprintf(p2, "%02d/%s/%d:%02d:%02d:%02d %c%02d%02d",
                       tm.tm_mday, months[tm.tm_mon - 1],
                       tm.tm_year, tm.tm_hour,
                       tm.tm_min, tm.tm_sec,
                       tp->gmtoff < 0 ? '-' : '+',
                       ptrx_abs(tp->gmtoff / 60),
                       ptrx_abs(tp->gmtoff % 60));

    p3 = &cached_http_log_iso8601[slot][0];

    (void)ptrx_sprintf(p3, "%4d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
                       tm.tm_year, tm.tm_mon,
                       tm.tm_mday, tm.tm_hour,
                       tm.tm_min, tm.tm_sec,
                       tp->gmtoff < 0 ? '-' : '+',
                       ptrx_abs(tp->gmtoff / 60),
                       ptrx_abs(tp->gmtoff % 60));

    ptrx_memory_barrier();

    ptrx_cached_time = tp;
    ptrx_cached_http_time.data = p0;
    ptrx_cached_err_log_time.data = p1;
    ptrx_cached_http_log_time.data = p2;
    ptrx_cached_http_log_iso8601.data = p3;

    ptrx_unlock(&ptrx_time_lock);

};
