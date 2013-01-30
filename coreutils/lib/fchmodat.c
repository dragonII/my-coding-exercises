/* Change the protections of file relative to an open directory. */

#include <sys/stat.h>
#include <errno.h>

#ifndef HAVE_LCHMOD
/* Use a different name, to avoid conflicting with any
   system-supplied declaration */
# undef lchmod
# define lchmod lchmod_rpl
int
lchmod(char* f, mode_t m)
{
    errno = ENOSYS;
    return -1;
}
#endif
