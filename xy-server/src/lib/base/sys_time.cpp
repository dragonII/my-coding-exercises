#include "sys_types.h"
#include "sys_time.h"
#include "log_wrap.h"
#include <signal.h>
#include <errno.h>
#include <string.h>

namespace TimeUtils
{
    namespace detail
    {
        struct TimeSpec
        {
            unsigned int year; // from 2010
            unsigned int mon;
            unsigned int day;
            unsigned int hour;
            unsigned int min;
            unsigned int sec;
        };

        union TimeU
        {
            time_spec_type  spec;
            TimeSpec        timeSpec;
        };
    };

    time_spec_type GetLocalTimeSpec()
    {
        time_t tCurrTime = GetCurrentTimeVal.tv_sec;
        tm* tmNow = localtime(&tCurrTime);
        detail::TimeU spec;
        spec.timeSpec.year  = tmNow->tm_year - (2010 - 1900);
        spec.timeSpec.mon   = tmNow->tm_mon;
        spec.timeSpec.day   = tmNow->tm_mday;
        spec.timeSpec.hour  = tmNow->tm_hour;
        spec.timeSpec.min   = tmNow->tm_min;
        spec.timeSpec.sec   = tmNow->tm_sec;

        return spec.spec;
    }
    static TickData tmp;
    TickData* gTickData = &tmp;

};
