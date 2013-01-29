/* inttostr.c -- convert integers to printable strings */

#include "inttostr.h"
#include "verify.h"

/* Convert I to a printable string in BUF, which must be at least
   INT_BUFSIZE_BOUND (INTTYPE) bytes long. Return the address of the
   printable string, which need not start at BUF. */

char* inttostr(inttype i, char* buf)
{
    char* p = buf + INT_STRLEN_BOUND (inttype);
    *p = 0;

    verify(TYPE_SIGNED (inttype) == inttype_is_signed);
#if inttype_is_signed
    if(i < 0)
    {
        do
            *--p = '0' - i % 10;
        while((i /= 10) != 0);

        *--p = '-';
    }
    else
#endif
    {
        do
            *--p = '0' + i % 10;
        while((i /= 10) != 0);
    }
    return p;
}
