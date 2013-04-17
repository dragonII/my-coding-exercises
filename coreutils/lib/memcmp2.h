#ifndef MEMCMP2_H
#define MEMCMP2_H

#include <stddef.h>

/* Compare the memory regions S1 = [s1..s1+N1-1], S2 = [s2..s2+N2-1],
   lexicographically.
   This function's result is locale independent, unlike memcoll()'s.
   Return a negative number if S1 < S2, a positive number if S1 > S2,
   or 0 if S1 and S2 have the same contents */
int memcmp2(const char *s1, size_t n1, const char *s2, size_t n2);


#endif
