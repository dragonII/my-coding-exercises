/* Locale-specific memory comparison */

#include "memcoll.h"

#include <errno.h>
#include <string.h>

/* Compare S1 (with length S1LEN) and S2 (with length S2LEN) according 
   to the LC_COLLATE locale. S1 and S2 do not overlap, and are not
   adjacent. Perhaps temporarily modify the bytes after S1 and S2,
   but restore their original contents before returning. Set errno to an
   errno number if there is an error, and to zero otherwise */
int memcoll(char *s1, size_t s1len, char *s2, size_t s2len)
{
    int diff;

    diff = memcmp(s1, s2, s1len < s2len ? s1len : s2len);
    if(!diff)
        diff = s1len < s2len ? -1 : s1len != s2len;
    errno = 0;

    return diff;
}
