#ifndef __PTRX_PALLOC_H__
#define __PTRX_PALLOC_H__

#include <ptrx_buf.h>
#include <ptrx_log.h>

typedef struct ptrx_pool_s          ptrx_pool_t;

typedef struct
{
    unsigned char       *last;
    unsigned char       *end;
    ptrx_pool_t         *next;
    unsigned int        failed;
} ptrx_pool_data_t;

typedef struct ptrx_pool_large_s ptrx_pool_large_t;
struct ptrx_pool_large_s
{
    ptrx_pool_large_t   *next;
    void                *alloc;
};

typedef void (*ptrx_pool_cleanup_pt)(void *data);
typedef struct ptrx_pool_cleanup_s  ptrx_pool_cleanup_t;
struct ptrx_pool_cleanup_s
{
    ptrx_pool_cleanup_pt    handler;
    void                    *data;
    ptrx_pool_cleanup_t     *next;
};

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

ptrx_pool_t *ptrx_create_pool(size_t size, ptrx_log_t *log);

void        *ptrx_pnalloc(ptrx_pool_t *pool, size_t size);


#endif
