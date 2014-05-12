#ifndef __PTRX_FILE_H_INCLUDED__
#define __PTRX_FILE_H_INCLUDED__

#define PTRX_FILE_RDONLY          O_RDONLY
#define PTRX_FILE_WRONLY          O_WRONLY
#define PTRX_FILE_RDWR            O_RDWR
#define PTRX_FILE_CREATE_OR_OPEN  O_CREAT
#define PTRX_FILE_OPEN            0
#define PTRX_FILE_TRUNCATE        O_CREAT|O_TRUNC
#define PTRX_FILE_APPEND          O_WRONLY|O_APPEND
#define PTRX_FILE_NONBLOCK        O_NONBLOCK

#define PTRX_FILE_DEFAULT_ACCESS  0644
#define PTRX_FILE_OWNER_ACCESS    0600

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <string/ptrx_string.h>

typedef struct stat     ptrx_file_info_t;

typedef struct ptrx_open_file_s 
{
    int                 fd;
    ptrx_str_t          name;
    ptrx_file_info_t    info;
} ptrx_open_file_t;

#define ptrx_open_file(name, mode, create, access)      \
     open((const char *)name, mode|create, access)

#endif
