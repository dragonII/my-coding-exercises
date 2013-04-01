#include "file_.h"

#include <sys/types.h>
#include <string.h>

/* Copy src to string dst of size siz. At most siz - 1 characters
   will be copied. Always NUL terminates (unless siz == 0).
   Return strlen(src); if retval >= siz, truncation occurred */
size_t strlcpy(char* dst, const char* src, size_t siz)
{
    char* d = dst;
    const char* s = src;
    size_t n = siz;

    /* copy as many bytes as will fit */
    if(n != 0 && --n != 0)
    {
        do
        {
            if((*d++ = *s++) == 0)
                break;
        } while(--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if(n == 0)
    {
        if(siz != 0)
            *d = '\0';      /* Null terminate dst */
        while(*s++)
            ;
    }

    return (s - src - 1);       /* count doest not include NULL */
}
