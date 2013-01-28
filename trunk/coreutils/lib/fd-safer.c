/* Return a safer copy of a file descriptor */

#include <errno.h>
#include <unistd.h>

#include "unistd-safer.h"

/* Return FD, unless FD would be a copy of standard input, output, or
   error; in that case, return a duplicate of FD, closing FD. On
   failure to duplicate, close FD, set errno, and return -1. Preserve
   errno if FD is negative, so that the caller can always inspect
   errno when the returned value is negative.

   This function is usefully wrapped around functions that return file
   descriptors, e.g., fd_safer(open("file", O_RDONLY)). */
int fd_safer(int fd)
{
    if(fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    {
        int f = dup_safer(fd);
        int e = errno;
        close(fd);
        errno = e;
        fd = f;
    }

    return fd;
}
