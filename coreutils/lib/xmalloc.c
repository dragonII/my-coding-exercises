/* xmalloc.c -- malloc with out of memory checking */

#include <stdlib.h>
#include <string.h>

#include "xalloc.h"

/* Change the size of an allocated block of memory P to N bytes,
   with error checking. */
void* xrealloc(void* p, size_t n)
{
    p = realloc(p, n);
    if(!p && n != 0)
        xalloc_die();
    return p;
}

/* If P is null, allocate a block of at least *PN bytes; otherwise,
   reallocate P so that it contains more than *PN bytes. *PN must be
   nonzero unless P is null. Set *PN to the new block's size, and
   return the pointer to the new block. *PN is never set to zero, and
   the returned pointer is never null. */
void* x2realloc(void *p, size_t *pn)
{
    return x2nrealloc(p, pn, 1);
}

/* Allocates N bytes of memory dynamically, with error checking */
void* xmalloc(size_t n)
{
    void* p = malloc(n);
    if(!p && n != 0)
        xalloc_die();
    return p;
}

/* Clone an object P os size S, with error checking. There's no need
   for xnmemdup(P, N, S), since xmemdup(P, N * S) works without any
   need for an arithmetic overflow check. */
void* xmemdup(void* p, size_t s)
{
    return memcpy(xmalloc(s), p, s);
}

/* Clone string */
char* xstrdup(char* string)
{
    return xmemdup(string, strlen(string) + 1);
}
