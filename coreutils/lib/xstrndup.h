/* Duplicate a bounded initial segment of a string, with out-of-memory
   checking */

#ifndef XSTRNDUP_H
#define XSTRNDUP_H

#include <stddef.h>

/* Return a newly allocated copy of at most N bytes of STRING.
   In other words, return a copy of the initial segment of length N of
   STRING. */
char* xstrndup(const char* string, size_t n);


#endif
