/* memory allocation routines with error checking. */

#include "libiberty.h"

#include <unistd.h>

void xmalloc_set_program_name(const char *s)
{
    name = s;
    if(first_break == NULL)
        first_break = (char *)sbrk(0);
}
