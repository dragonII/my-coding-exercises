/* xexit.c -- Run any exit handlers, then exit */

#include <stdio.h>
#include <stdlib.h>

#include "libiberty.h"

/* This variable is set by xatexit if it is called. This way, xmalloc
   doesn't drag xatexit into the link */
void (*_xexit_cleanup)(void);

void xexit(int code)
{
    if(_xexit_cleanup != NULL)
        (*_xexit_cleanup)();
    exit(code);
}
