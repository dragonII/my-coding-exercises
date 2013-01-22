/* A more useful interface to strtol */


#ifndef __strtol
# define __strtol   strtol
# define __strtol_t long int
# define __xstrtol  xstrtol
# define STRTOL_T_MINIMUM LONG_MIN
# define STRTOL_T_MAXIMUM LONG_MAX
#endif

#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "intprops.h"
#include "xstrtol.h"

strtol_error __xstrtol(char*s, char** ptr, int strtol_base,
                        __strtol_t* val, char* valid_suffixes)
{
    char* t_ptr;
    char** p;
    __strtol_t tmp;
    strtol_error err = LONGINT_OK;

    assert(strtol_base >= 0 && strtol_base <= 36);

    p = (ptr ? ptr : &t_ptr);

    if(! TYPE_SIGNED(__strtol_t))
    {
        char* q = s;
        unsigned char ch = *q;
        while(isspace(ch))
            ch = *++q;
        if(ch == '-')
            return LONGINT_INVALID;
    }

    errno = 0;
    tmp = __strtol(s, p, strtol_base);

    if(*p == s)
    {
        /* If there is no number but there is a valid suffix, assume the
           number is 1. The string is invalid otherwise. */
        if(valid_suffixes && **p && strchr(valid_suffixes, **p))
            tmp = 1;
        else
            return LONGINT_INVALID;
    }
    else if(errno != 0)
    {
        if(errno != ERANGE)
            return LONGINT_INVALID;
        err = LONGINT_OVERFLOW;
    }

    /* Let valid_suffixes == NULL mean `allow any suffix' */
    if(!valid_suffixes)
    {
        *val = tmp;
        return err;
    }

    if(**p != '\0')
    {
        int base = 1024;
        int suffixes = 1;
        strtol_error overflow;

        if(!strchr(valid_suffixes, **p))
        {
            *val = tmp;
            return err | LONGINT_INVALID_SUFFIX_CHAR;
        }
        if(strchr(valid_suffixes, '0'))
        {
            /* The ``valid_suffix'' '0' is a special flag meaning that
               an optional second suffix is allowed, which can change
               the base. A suffix "B" (e.g. "100MB") stands for a power
               of 1000, whereas a suffix "iB" (e.g. "100MiB") stands for
               a power of 1024. If no suffix (e.g. "100M"), assume
               power-of-1024 */
