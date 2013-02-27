/* closexec.c - set or clear the close-on-exec descriptor flag */

#include "closexec.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

/* Set the `FD_CLOEXEC' flag of DESC if VALUE is true,
   or clear the flag if VALUE is false.
   Return 0 on success, or -1 on error with `errno' set.

   Note that on MingW, this function does NOT protect DESC from being
   inherited into spawned children. Instead, either use dup_cloexec
   followed by closing the original DESC, or use interfaces such as
   open or pipe2 that accept flags like O_CLOEXEC to create DESC
   non-inheritable in the first place. */
int set_closexec_flag(int desc, bool value)
{
#ifdef F_SETFD
    
    int flags = fcntl(desc, F_GETFD, 0);

    if(flags >= 0)
    {
        int newflags = (value ? flags | FD_CLOEXEC : flags & ~FD_CLOEXEC);

        if(flags == newflags
             || fcntl(desc, F_SETFD, newflags) != -1)
            return 0;
    }

    return -1;

#else  /* No F_SETFD */

    /* Use dup2 to reject invalid file descriptors; the closexec flag
       will be unaffected. */
    if(desc < 0)
    {
        errno = EBADF;
        return -1;
    }
    if(dup2(desc, desc) < 0)
        /* errno is EBADF here */
        return -1;

    /* There is nothing we can do on this kind of platform, Punt */
    return 0;

#endif
}
