#ifndef __PTRX_LIST_H__
#define __PTRX_LIST_H__

typedef struct ptrx_list_part_s ptrx_list_part_t;

struct ptrx_list_part_s
{
    void                *elts;
    unsigned int        nelts;
    ptrx_list_part_t    *next;
};

typedef struct
{
    ptrx_list_part_t    *last;
    ptrx_list_part_t    part;
    size_t              size;
    unsigned int        nalloc;
    ptrx_pool_t         *pool;
} ptrx_list_t;


#endif
