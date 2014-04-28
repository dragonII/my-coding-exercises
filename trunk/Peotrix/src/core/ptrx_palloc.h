#ifndef __PTRX_PALLOC_H__
#define __PTRX_PALLOC_H__

#include <ptrx_core.h>

typedef struct
{
    unsigned char       *last;
    unsigned char       *end;
    ptrx_pool_t         *next;
    unsigned int        failed;
} ptrx_pool_data_t;

struct ptrx_pool_s
{
    ptrx_pool_data_t        d;
    size_t                  max;
    ptrx_pool_t             *current;
    ptrx_chain_t            *chain;
    ptrx_pool_large_t       *large;
    ptrx_pool_cleanup_t     *cleanup;
    ptrx_log_t              *log;
};



#endif
