/* Invoke dup, but avoid some glitches */

#include "unistd-safer.h"

#include <fcntl.h>
#include <unistd.h>

/* Like dup, but do not return STDIN_FILENO, STDOUT_FILENO, or
   STDERR_FILENO */
int dup_safer(int fd)
{
    return fcntl(fd, F_DUPFD, STDERR_FILENO + 1);
}
