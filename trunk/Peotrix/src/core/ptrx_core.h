#ifndef __PTRX_CORE_H__
#define __PTRX_CORE_H__

#include <ptrx_string.h>

typedef struct ptrx_open_file_s     ptrx_open_file_t;
typedef struct ptrx_log_s           ptrx_log_t;
typedef struct ptrx_cycle_s         ptrx_cycle_t;
typedef struct ptrx_connection_s    ptrx_connection_t;
typedef struct ptrx_array_s         ptrx_array_t;
typedef struct ptrx_pool_s          ptrx_pool_t;

#define PTRX_OK         0
#define PTRX_ERROR      -1
#define PTRX_AGAIN      -2
#define PTRX_BUSY       -3
#define PTRX_DONE       -4
#define PTRX_DECLINED   -5
#define PTRX_ABORT      -6

#endif
