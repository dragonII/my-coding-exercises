/* xgethostname.c -- return current hostname with unlimited length */

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "xgethostname.h"
#include "xalloc.h"


#ifndef INITIAL_HOSTNAME_LENGTH
# define INITIAL_HOSTNAME_LENGTH 34
#endif

/* Return the current hostname in malloc'd storage.
   If malloc fails, exit.
   Upon any other failures, return NULL and set errno. */
char* xgethostname(void)
{
    char* hostname = NULL;
    size_t size = INITIAL_HOSTNAME_LENGTH;

    while(1)
    {
        /* Use SIZE_1 here rather than SIZE to work around the bug in
           SunOS 5.5's gethostname whereby it NULL-terminates HOSTNAME
           even when the name is as long as the supplied buffer. */
        size_t size_1;

        hostname = x2realloc(hostname, &size);
        size_1 = size - 1;
        hostname[size_1 - 1] = '\0';
        errno = 0;

        if(gethostname(hostname, size_1) == 0)
        {
            if(!hostname[size_1 - 1])
                break;
        }
        else if(errno != 0 && errno != ENAMETOOLONG && errno != EINVAL
                /* OSX/Dawin does this when the buffer is not large enough*/
                && errno != ENOMEM)
        {
            int saved_errno = errno;
            free(hostname);
            errno = saved_errno;
            return NULL;
        }
    }
    return hostname;
}
