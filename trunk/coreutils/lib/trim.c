#include "trim.h"
#include "xalloc.h"

#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

char* trim2(const char* s, int how)
{
    char *d;

    d = strdup(s);

    if(!d)
        xalloc_die();

    char *p;

    /* Trim leading whitespaces. */
    if(how != TRIM_TRAILING)
    {
        for(p = d; *p && isspace((unsigned char)*p); p++)
            ;
        memmove(d, p, strlen(p) + 1);
    }
    /* Trim trailing whitespaces. */
    if(how != TRIM_LEADING)
    {
        for(p = d + strlen(d) - 1; p >= d && isspace((unsigned char)*p); p--)
            *p = '\0';
    }
    return d;
}

