/* xgetcwd.c -- return current directory with unlimited length */

#include "xgetcwd.h"

#include <errno.h>
#include <unistd.h>

#include "xalloc.h"

/* Return the current directory, newly allocated.
   Upon an out-of-memory error, call xalloc_die.
   Upon any other type of error, return NULL. */
char* xgetcwd(void)
{
    char* cwd = getcwd(NULL, 0);
    if(!cwd && errno == ENOMEM)
        xalloc_die();
    return cwd;
}
