/* bitrotate.h - Rotate bits in integers */

#ifndef _GL_BITROTATE_h
#define _GL_BITROTATE_h

#include <sys/types.h>
#include <limits.h>
#include <stdint.h>

/* Given a size_t argument X, return the value corresponding
   to rotating the bits N steps to the right. N must be between 1 to
   (CHAR_BIT * sizeof(size_t) - 1) inclusive. */
static inline size_t
rotr_sz(size_t x, int n)
{
    return ((x >> n) | (x << ((CHAR_BIT * sizeof x) -n ))) & SIZE_MAX;
}


#endif
