#ifndef __PTRX_ARRAY_H__
#define __PTRX_ARRAY_H__

#include <stddef.h>

#include <ptrx_palloc.h>

typedef struct ptrx_array_s         ptrx_array_t;

struct ptrx_array_s
{
    void            *elts;
    unsigned int    nelts;
    size_t          size;
    unsigned int    nalloc;
    ptrx_pool_t     *pool;
};

static inline int
ptrx_array_init(ptrx_array_t *array, ptrx_pool_t *pool, 
                unsigned int n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC
     * thinks that "array-nelts" may be used without initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = ptrx_palloc(pool, n * size);
    if(array->elts == NULL)
    {
        return PTRX_ERROR;
    }

    return PTRX_OK;
}

void *ptrx_array_push(ptrx_array_t *a);

#endif
