#ifndef __PTRX_ARRAY_H__
#define __PTRX_ARRAY_H__

#include <stddef.h>

struct ptrx_array_s
{
    void            *elts;
    unsigned int    nelts;
    size_t          size;
    unsigned int    nalloc;
    ptrx_pool_t     *pool;
};


#endif
