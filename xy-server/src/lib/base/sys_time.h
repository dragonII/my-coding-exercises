#ifndef __LIB_SYS_TIMER_H__
#define __LIB_SYS_TIMER_H__

#include <time.h>
#include <sys/time.h>

#include "sys_types.h"

namespace TimeUtils
{
    const long C_TICK_COUNT_PER_SECOND = 20;
    const char* const TIME_FORMAT = "%Y-%m-%d %H:%M:%S";
#define ONE_SECOND_IN_MSECS  1000L
#define ONE_SECOND_IN_USECS  1000000L
#define ONE_MSECOND_IN_USECS 1000L

    inline void TimeToString(long _tmpStamp, char* _timeStr, int _length)
    {
        time_t t = time_t(_tmpStamp);
        struct tm* pTmpTime = localtime(&t);
        strftime(_timeStr, _length, TIME_FORMAT, pTmpTime);
    }

    inline long StringToTime(const char* _timeStr)
    {
        struct tm tmpTime;
        strptime(_timeStr, TIME_FORMAT, &tmpTime);
        return long(mktime(&tmpTime));
    }

    inline long TickToMs(long tick)
    {
        return tick * (ONE_SECOND_IN_MSECS / C_TICK_COUNT_PER_SECOND);
    }

    inline long MsToTick(long ms)
    {
        return ms * (C_TICK_COUNT_PER_SECOND / ONE_SECOND_IN_MSECS);
    }

    inline long TickToSec(long tick)
    {
        return tick / C_TICK_COUNT_PER_SECOND;
    }

    inline long SecToTick(long sec)
    {
        return sec * C_TICK_COUNT_PER_SECOND;
    }

    time_spec_type GetLocalTimeSpec();

    inline void GetRealTime(timeval* _val)
    {
        gettimeofday(_val, 0);
    }

    struct TickData
    {
        TickData();
        timeval systemStart;
        timeval currentTimeVal;
        long    currentTick;
    };
    extern TickData* gTickData;

    template<typename TickProc>
    long Tick(TickProc _proc)
    {
        timeval tmNow;
        GetRealTime(&tmNow);
        int count = 0;
        if(tmNow >= gTickData->currentTimeVal)
        {
            gTickData->currentTimeVal = tmNow;
            long tickNow = MsToTick((gTickData->currentTimeVal - gTickData->systemStart) / ONE_MSECOND_IN_USECS);
            while(gTickData->currentTick < tickNow)
            {
                ++gTickData->currentTick;
                count += _proc(gTickData->currentTick);
            }
        }
        return count;
    }

    template<typename _Ret, typename _Tp, typename _Arg>
    class tick_mem_fun1_t
    {
    public:
        typedef _Arg argument_type;
        typedef _Ret result_type;
        explicit tick_mem_fun1_t(_Ret (_Tp::*__pf)(_Arg), _Tp* _p)
                    : _M_f(__pf), object_(_p) {}

        _Ret operator()(_Arg __x) const
        {
            return (object_->*_M_f)(__x);
        }

    private:
        _Ret (_Tp::*_M_f)(_Arg);
        _Tp* object_;
    };

    template<typename _Ret, typename _Tp, typename _Arg>
    inline tick_mem_fun1_t<_Ret, _Tp, _Arg>
        tick_mem_fun(_Ret (_Tp::*__f)(_Arg), _Tp* _p)
        {
            return tick_mem_fun1_t<_Ret, Tp, _Arg>(__f, _p);
        }

};

#define GetCurrentTick()    TimeUtils::gTickData->currentTick
#define GetCurrentTimeVal() TimeUtils::gTickData->currentTimeVal



#endif
