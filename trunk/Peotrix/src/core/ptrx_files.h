#ifndef __PTRX_FILES_H__
#define __PTRX_FILES_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ptrx_core.h>
#include <ptrx_string.h>

typedef int             ptrx_fd_t;
typedef struct stat     ptrx_file_info_t;

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


/*
 * we use inlined function instead of simple #define
 * because glibc 2.3 sets warn_unused_result attribute for write()
 * and in this case gcc 4.3 ignores (void) cast
 */
static inline ssize_t
ptrx_write_fd(int fd, void *buf, size_t n)
{
    return write(fd, buf, n);
}

typedef struct ptrx_file_s  ptrx_file_t;
struct ptrx_file_s
{
    ptrx_fd_t           fd;
    ptrx_str_t          name;
    ptrx_file_info_t    info;

    off_t               offset;
    off_t               sys_offset;

    ptrx_log_t          *log;

#if PTRX_HAVE_FILE_AIO
    ptrx_event_aio_t    *aio;
#endif

    unsigned            valid_info:1;
    unsigned            directio:1;
};


//typedef struct ptrx_open_file_s ptrx_open_file_t;
//struct ptrx_open_file_s
//{
//    int             fd;
//    ptrx_str_t      name;
//    unsigned char   *buffer;
//    unsigned char   *pos;
//    unsigned char   *last;
//};



#endif
