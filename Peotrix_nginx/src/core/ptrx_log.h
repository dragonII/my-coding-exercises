#ifndef __PTRX_LOG_H__
#define __PTRX_LOG_H__


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

#include <ptrx_errno.h>
#include <ptrx_string.h>

typedef struct ptrx_log_s   ptrx_log_t;

typedef unsigned char *(*ptrx_log_handler_pt)(ptrx_log_t *log, unsigned char *buf, size_t len);

typedef struct ptrx_open_file_s ptrx_open_file_t;
struct ptrx_open_file_s
{
    int             fd;
    ptrx_str_t      name;
    unsigned char   *buffer;
    unsigned char   *pos;
    unsigned char   *last;
};


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

ptrx_log_t *ptrx_log_init(unsigned char *prefix);
void        ptrx_log_stderr(ptrx_err_t err, const char *fmt, ...);
void        ptrx_log_error(unsigned int level, ptrx_log_t *log, 
                           ptrx_err_t err, const char *fmt, ...);


#define PTRX_MAX_ERROR_STR  2048

/*
 * ptrx_write_stderr() cannot be implemented as macro, since
 * MSVC does allow to use #ifdef inside macro parameters.
 *
 * ptrx_write_fd() is used to instead of ptrx_write_console(), since
 * CharToOemBuff() inside ptrx_write_console() cannot be used with
 * read only buffer as destination and CharToOemBuff() is not needed
 * for ptrx_write_stderr() anyway.
 */

#include <ptrx_files.h>
#include <string.h>

//unsigned int    ptrx_use_stderr = 1;

static inline void
ptrx_write_stderr(char *text)
{
    (void) ptrx_write_fd(ptrx_stderr, text, strlen(text));
}


#endif
