#ifndef __PTRX_LOG_H_INCLUDED__
#define __PTRX_LOG_H_INCLUDED__

#define PTRX_LOG_STDERR             0
#define PTRX_LOG_EMERG              1
#define PTRX_LOG_ALERT              2
#define PTRX_LOG_CRIT               3
#define PTRX_LOG_ERR                4
#define PTRX_LOG_WARN               5
#define PTRX_LOG_NOTICE             6
#define PTRX_LOG_INFO               7
#define PTRX_LOG_DEBUG              8


#define PTRX_LOG_FILE_PATH          "/usr/local/PeoTrix/log/peotrix.log"
#define PTRX_LOG_MAX_SIZE           10485760

#define PTRX_MAX_ERR_STR            2048

#include <file/ptrx_file.h>

typedef struct ptrx_log_s
{
    int                 log_level;
    ptrx_open_file_t    file;
    unsigned int        connection;
} ptrx_log_t ;


int     ptrx_log_init(ptrx_log_t *log);

/* output log to stderr */
void    ptrx_log_stderr(ptrx_log_t *log, int level,
                        int errno, char *fmt, ...);

/* output log to file */
void    ptrx_log_error(ptrx_log_t *log, int level,
                       int  errno, char *fmt, ...);



#endif
