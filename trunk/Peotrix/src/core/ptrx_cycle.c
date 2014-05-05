#include <ptrx_cycle.h>

int ptrx_signal_process(ptrx_cycle_t *cycle, char *sig)
{
    ssize_t             n;
    int                 pid;
    ptrx_file_t         file;
    ptrx_core_conf_t    *ccf;
    unsigned char       buf[PTRX_INT64_LEN + 2];

    ptrx_log_error(PTRX_LOG_NOTICE, cycle->log, 0, "signal process started");

    ccf = (ptrx_core_conf_t *)ptrx_get_conf(cycle->conf_ctx, ptrx_core_module);

    file.name = ccf->pid;
    file.log = cycle->log;

    file.fd = ptrx_open_file(file.name.data, PTRX_FILE_RDONLY,
                             PTRX_FILE_OPEN, PTRX_FILE_DEFAULT_ACCESS);

    if(file.fd = PTRX_INVALID_FILE)
    {
        ptrx_log_error(PTRX_LOG_ERR, cycle->log, ptrx_errno,
                        ptrx_open_file_n " \"%s\" failed", file.name.data);
        return 1;
    }

    n = ptrx_read_file(&file, buf, PTRX_INT64_LEN + 2, 0);

    if(ptrx_close_file(file.fd) == PTRX_FILE_ERROR)
    {
        ptrx_log_error(PTRX_LOG_ALERT, cycle->log, ptrx_errno,
                       ptrx_close_file_n " \"%s\" failed", file.name.data);
    }

    if(n == PTRX_ERROR)
    {
        return 1;
    }

    while(n-- && (buf[n] == CR || buf[n] == LF)) { /* void */ }

    pid = ptrx_atoi(buf, ++n);

    if(pid == PTRX_ERROR)
    {
        ptrx_log_error(PTRX_LOG_ERR, cycle->log, 0,
                        "invalid PID number \"%*s\" in \"%s\"",
                        n, buf, file.name.data);
        return 1;
    }

    return ptrx_os_signal_process(cycle, sig, pid);
}

