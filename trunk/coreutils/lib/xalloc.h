/* xalloc.h -- malloc with out-of-memory checking */

#ifndef XALLOC_H
#define XALLOC_H

#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>

/* This function is always triggered when memory is exhausted.
   It must be defined by the application, either explicitly
   or by using gnulib's xalloc-die module. This is the
   function to call when one wants the program to die because of a
   memory allocation failure. */
void xalloc_die(void);

void* xmalloc(size_t s);
void* xrealloc(void* p, size_t s);
void* x2realloc(void* p, size_t* pn);
void* xmemdup(void* p, size_t s);
char* xstrdup(char* str);

/* Allocate memory for N elemants of type T, with error checking */
#define XNMALLOC(n, t) \
    ((t *) (sizeof (t) == 1 ? xmalloc (n) : xnmalloc (n, sizeof(t))))

/* Return 1 if an array of N objects, each of size S, cannot exist due
   to size arithmetic overflow. S must be positive and N must be
   nonnegative. This is a macro, not an inline function, so that it
   works correctly even when SIZE_MAX < N.
   By gnulib convention, SIZE_MAX represents overflow in size
   calculations, so the conservative dividend to use here is
   SIZE_MAX - 1, since SIZE_MAX might represent an overflowed value.
   However, malloc(SIZE_MAX) failes on all known hosts where
   sizeof(ptrdiff_t) <= sizeof(size_t), so do not bother to test for
   exactly-SIZE_MAX allocations on such hosts; this avoids a test and
   branch when S is known to be 1. */
#define xalloc_oversized(n, s) \
   ((size_t) (sizeof(ptrdiff_t) <= sizeof(size_t) ? -1 : -2) / (s) < (n))

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

static inline void*
xnmalloc(size_t n, size_t s)
{
    if(xalloc_oversized(n, s))
        xalloc_die();
    return xmalloc(n * s);
}


/* Return a pointer to a new buffer of N bytes. This is like xmalloc,
   except it returns char* */
static inline char*
xcharalloc(size_t n)
{
    return XNMALLOC(n, char);
}



#endif
