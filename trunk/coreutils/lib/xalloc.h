/* xalloc.h -- malloc with out-of-memory checking */

#ifndef XALLOC_H
#define XALLOC_H

#include <sys/types.h>

/* This function is always triggered when memory is exhausted.
   It must be defined by the application, either explicitly
   or by using gnulib's xalloc-die module. This is the
   function to call when one wants the program to die because of a
   memory allocation failure. */
void xalloc_die(void);

void* xrealloc(void* p, size_t s);
void* x2realloc(void* p, size_t* pn);

static inline void*
x2nrealloc(void* p, size_t* pn, size_t s)
{
    size_t n = *pn;
    if(!p)
    {
        if(!n)
        {
            /* The approximate size to use for initial small allocation
               requests, when the invoking code specifies an old size of
               zero. 64 bytes is the largest "small" request for the
               GNU C library malloc. */
            enum { DEFAULT_MXFAST = 64 };

            n = DEFAULT_MXFAST / s;
            n += !n;
        }
    }
    else
    {
        /* Set N = ceil (1.5 * N) so that progress is made if N == 1.
           Check for overflow, so that N * S stays in size_t range.
           The check is slightly conservative, but an exact check isn't
           worth the trouble. */
        if((size_t) -1 / 3 * 2 / s <= n)
            xalloc_die();
        n += (n + 1) / 2;
    }

    *pn = n;
    return xrealloc(p, n * s);
}



#endif
