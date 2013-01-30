/* An interface to read and write that retries after interrupts */

/* Specification. */
#ifdef SAFE_WRITE
# include "safe-write.h"
#else
# include "safe-read.h"
#endif

/* Get ssize_t */
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>

#ifdef EINTR
# define IS_EINTR(x) ((x) == EINTR)
#else
# define IS_EINTR(x) 0
#endif

#include <limits.h>

#ifdef SAFE_WRITE
# define safe_rw safe_write
# define rw write
#else
# define safe_rw safe_read
# define rw read
# undef const
# define const /* empty */
#endif

/* Read(write) up to COUNT bytes at BUF from(to) descriptor FD, retrying if
   interrupted. Return the actual number of bytes read(written), zero for EOF,
   of SAFE_READ_ERROR(SAFE_WRITE_ERROR) upon error. */
size_t safe_rw(int fd, void const* buf, size_t count)
{
    /* Work around a bug in Tru64 5.1. Attempting to read more than
       INT_MAX bytes fails with errno = EINVAL. When decreasing COUNT,
       keep it block-aligned */
    enum { BUGGY_READ_MAXIMUM = INT_MAX & ~8191 };

    for(;;)
    {
        ssize_t result = rw(fd, buf, count);

        if(result >= 0)
            return result;
        else if(IS_EINTR(errno))
            continue;
        else if(errno == EINVAL && BUGGY_READ_MAXIMUM < count)
            count = BUGGY_READ_MAXIMUM;
        else
            return result;
    }
}
