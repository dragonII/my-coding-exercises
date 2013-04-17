#include "memcmp2.h"

#include <string.h>

int memcmp2(const char *s1, size_t n1, const char *s2, size_t n2)
{
    int cmp = memcmp(s1, s2, n1 <= n2 ? n1 : n2);
    if(cmp == 0)
    {
        if(n1 < n2)
            cmp = -1;
        else if(n1 > n2)
            cmp = 1;
    }

    return cmp;
}
