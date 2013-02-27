/* convert string representation of a number into an intmax_t value */

#ifndef STRTOIMAX_HEADER
#define STRTOIMAX_HEADER

#include <inttypes.h>
#include <stdlib.h>

inline int strtoimax(char* ptr, char** endptr, int base)
{
    return strtol(ptr, endptr, base);
}


#endif
