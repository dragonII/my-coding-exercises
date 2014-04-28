#ifndef __PTRX_LOG_H__
#define __PTRX_LOG_H__

#include <ptrx_config.h>

#define PTRX_LOG_STDERR      0
#define PTRX_LOG_EMERG       1
#define PTRX_LOG_ALERT       2
#define PTRX_LOG_CRIT        3
#define PTRX_LOG_ERR         4
#define PTRX_LOG_WARN        5
#define PTRX_LOG_NOTICE      6
#define PTRX_LOG_INFO        7
#define PTRX_LOG_DEBUG       8

#define PTRX_ERROR_LOG_PATH "log/error.log"

typedef unsigned char *(ptrx_log_handler_pt)(ptrx_log_t *log, unsigned char *buf, size_t len);
void ptrx_log_stderr(ptrx_err_t err, const char *fmt, ...);


struct ptrx_log_s
{
    unsigned int        log_level;
    ptrx_open_file_t    *file;
    unsigned int        connection;

    ptrx_log_handler_pt handler;
    void                *data;

    /*
     * We declare "action" as "char *" because the actions are usually
     * the static strings and in the "unsigned char *" case we have to
     * override their types all the time.
     */
    char                *action;
};

#define PTRX_MAX_ERROR_STR  2048


#endif
