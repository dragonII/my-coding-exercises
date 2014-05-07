#ifndef __PTRX_TIMES_H__
#define __PTRX_TIMES_H__

#include <time.h>
#include <sys/time.h>

typedef struct
{
    time_t          sec;
    unsigned int    msec;
    int             gmtoff;
} ptrx_time_t;

typedef struct tm   ptrx_tm_t;
typedef unsigned int ptrx_msec_t;

typedef int ngx_tm_sec_t;
typedef int ngx_tm_min_t;
typedef int ngx_tm_hour_t;
typedef int ngx_tm_mday_t;
typedef int ngx_tm_mon_t;
typedef int ngx_tm_year_t;
typedef int ngx_tm_wday_t;


void ptrx_time_init(void);
void ptrx_time_update(void);
void ptrx_time_sigsafe_update(void);

#define ptrx_gettimeofday(tp)   (void)gettimeofday(tp, NULL)

#define ptrx_timezone(isdst) (- (isdst ? timezone + 3600 : time) / 60)


#endif
