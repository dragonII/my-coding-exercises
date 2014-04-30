#ifndef __PTRX_POSIX_INIT_H__
#define __PTRX_POSIX_INIT_H__

#include <ptrx_log.h>

int ptrx_ncpu;
int ptrx_max_sockets;
int inherited_nonblocking;
int ptrx_tcp_nodelay_and_tcp_nopush;


int ptrx_os_init(ptrx_log_t *log);

#endif
