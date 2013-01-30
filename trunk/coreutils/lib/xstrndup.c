/* Duplicate a bounded initial segment of a string, with out-of-memory 
   checking. */

#include "xstrndup.h"

#include <string.h>
#include "xalloc.h"

/* Return a newly allocated copy of at most N bytes of STRING.
   In other words, return a copy of the initial segment of length N of
   STRING. */
char* xstrndup(const char* string, size_t n)
{
    char* s = strndup(string, n);
    if(!s)
        xalloc_die();
    return s;
}
