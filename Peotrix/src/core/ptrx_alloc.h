#ifndef __PTRX_ALLOC_H__
#define __PTRX_ALLOC_H__

#include <ptrx_log.h>

void *ptrx_alloc(size_t size, ptrx_log_t *log);
void *ptrx_calloc(size_t size, ptrx_log_t *log);



#endif
