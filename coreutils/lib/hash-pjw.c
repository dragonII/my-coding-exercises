/* hash-pjw.c -- compute a hash value from a NUL-terminated string. */

#include <limits.h>
#include <sys/types.h>

#define SIZE_BITS (sizeof(size_t) * CHAR_BIT)

/* A hash function for NUL-terminated char* string using
   the method described by Bruno Haible.
   See http://www.haible.de/bruno/hashfunc.html */

size_t hash_pjw(void* x, size_t tablesize)
{
    char* s;
    size_t h = 0;

    for(s = x; *s; s++)
        h = *s + ((h << 9) | (h >> (SIZE_BITS - 9)));

    return h % tablesize;
}

