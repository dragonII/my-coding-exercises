#ifndef __PTRX_ERRNO_H__
#define __PTRX_ERRNO_H__

#include <errno.h>

#define ptrx_errno      errno

typedef int     ptrx_err_t;

unsigned int ptrx_strerror_init(void);


#endif
