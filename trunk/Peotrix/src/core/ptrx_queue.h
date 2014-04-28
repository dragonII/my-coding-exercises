#ifndef __PTRX_QUEUE_H__
#define __PTRX_QUEUE_H__

typedef struct ptrx_queue_s ptrx_queue_t;

struct ptrx_queue_s
{
    ptrx_queue_t    *prev;
    ptrx_queue_t    *next;
};

#endif
