#ifndef __PTRX_FILES_H__
#define __PTRX_FILES_H__

#include <ptrx_core.h>

#define ptrx_linefeed(p)    *p++ = LF;
#define PTRX_LINEFEED_SIZE  1
#define PTRX_LINEFEED       "\x0a"

#define ptrx_stderr     STDERR_FILENO

#define PTRX_FILE_RDONLY        O_RDONLY
#define PTRX_FILE_WRONLY        O_WRONLY
#define PTRX_FILE_RDWR          O_RDWR
#define PTRX_FILE_CREATE_OR_OPEN  O_CREAT
#define PTRX_FILE_OPEN          0
#define PTRX_FILE_TRUNCATE      O_CREAT|O_TRUNC
#define PTRX_FILE_APPEND        O_WRONLY|O_APPEND
#define PTRX_FILE_NONBLOCK      O_NONBLOCK

#define PTRX_FILE_DEFAULT_ACCESS  0644
#define PTRX_FILE_OWNER_ACCESS    0600

#define PTRX_INVALID_FILE   -1
#define PTRX_FILE_ERROR     -1

#define ptrx_path_separator(c)  ((c) == '/')

#define ptrx_open_file(name, mode, create, access)  \
    open((const char *)name, mode|create, access)

#endif
