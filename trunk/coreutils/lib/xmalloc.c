/* xmalloc.c -- malloc with out of memory checking */

#include <stdlib.h>
#include <string.h>

/* If P is null, allocate a block of at least *PN bytes; otherwise,
   reallocate P so that it contains more than *PN bytes. *PN must be
   nonzero unless P is null. Set *PN to the new block's size, and
   return the pointer to the new block. *PN is never set to zero, and
   the returned pointer is never null. */
void* x2realloc(void *p, size_t *pn)
{
    return x2nrealloc(p, pn, 1);
}
