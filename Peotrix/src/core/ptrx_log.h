#ifndef __PTRX_LOG_H__
#define __PTRX_LOG_H__

#include <ptrx_config.h>

typedef unsigned char *(ptrx_log_handler_pt)(ptrx_log_t *log, unsigned char *buf, size_t len);

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



#endif
