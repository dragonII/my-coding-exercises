/* Invoke open, but return either a desired file descriptor or -1. */

#include "fd-reopen.h"

#include <error.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* Open a file to a particular file descriptor. This is like standard
   `open', except it always returns DESIRED_FD if successful */
int fd_reopen(int desired_fd, char* file, int flags, mode_t mode)
{
    int fd = open(file, flags, mode);

    if(fd == desired_fd || fd < 0)
        return fd;
    else
    {
        int fd2 = dup2(fd, desired_fd);
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return fd2;
    }
}
