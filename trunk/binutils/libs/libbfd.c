/* Associated BFD support routines, only used internally */

#include "include/bfd_.h"
#include "include/libbfd.h"


bfd_uint64_t
bfd_getb64(const void *p)
{
    const bfd_byte *addr = p;
    bfd_uint64_t v;

    v  = addr[0]; v <<= 8;
    v |= addr[1]; v <<= 8;
    v |= addr[2]; v <<= 8;
    v |= addr[3]; v <<= 8;
    v |= addr[4]; v <<= 8;
    v |= addr[5]; v <<= 8;
    v |= addr[6]; v <<= 8;
    v |= addr[7]; 

    return v;
}
