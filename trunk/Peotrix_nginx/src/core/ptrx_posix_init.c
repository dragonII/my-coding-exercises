#include <ptrx_posix_init.h>

int ptrx_os_init(ptrx_log_t *log)
{
    unsigned int    n;

    ptrx_init_setproctitle(log);

    ptrx_pagesize = getpagesize();
    ptrx_cacheline_size = PTRX_CPU_CACHE_LINE;

    for(n = ptrx_pagesize; n >>= 1; ptrx_pagesize_shift++) { /*void*/ }

    if(ptrx_ncpu < 1)
    {
        ptrx_ncpu = 1;
    }

    ptrx_cpuinfo();

    if(getrlimit(RLIMIT_NOFILE, &rlmt) == -1)
    {
        ptrx_log_error(PTRX_LOG_ALERT, log, errno,
                        "getrlimit(RLIMIT_NOFILE) failed");
        return PTRX_ERROR;
    }

    ptrx_max_sockets = (int)rlmt.rlmt_cur;

    srandom(ptrx_time());

    return PTRX_OK;
}

void ptrx_os_status(ptrx_log_t *log)
{
    ptrx_log_error(PTRX_LOG_NOTICE, log, 0, PTRX_VER);

    ptrx_log_error(PTRX_LOG_NOTICE, log, 0,
                    "getrlimit(RLIMIT_NOFILE): %r:%r",
                    rlmt.rlim_cur, rlmt.rlim_max);
}
