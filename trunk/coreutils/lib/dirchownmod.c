/* Change the ownership and mode bits of a directory */

#include "dirchownmod.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "sys_stat.in.h"
#include "stat-macros.h"

#ifndef HAVE_FCHMOD
# define HAVE_FCHMOD 0
# undef fchmod
# define fchmod(fd, mode) (-1)
#endif

//static int lchmod(char* f, mode_t m)
//{
//    errno = ENOSYS;
//    return -1;
//}



/* Change the ownership and mode bits of a directory. If FD is
   nonnegative, it should be a file descriptor associated with the
   directory; close it before returning. DIR is the name of the 
   directory.

   If MKDIR_MODE is not (mode_t) -1, mkdir (DIR, MKDIR_MODE) has just
   been executed successfully with umask zero, so DIR should be a
   directory (not a symbolic link).

   First, set the file's owner to OWNER and group to GROUP, but leave
   the owner alone if OWNER is (uid_t) -1, and similarly for GROUP.

   Then, set the file's mode bits to MODE, except preserve any of the
   bits that correspond to zero bits in MODE_BITS. In other words,
   MODE_BITS is a mask that specifies which of the file's mode bits
   should be set or cleared. MODE should be a subset of MODE_BITS,
   which in trun should be subset of CHMOD_MODE_BITS.

   This implementation assumes the current umask is zero. 

   Return 0 if successful, -1 (setting errno) otherwise. Unsuccessful
   calls may be the chown but not the chmod */

int dirchownmod(int fd, char* dir, mode_t mkdir_mode,
                uid_t owner, gid_t group,
                mode_t mode, mode_t mode_bits)
{
    struct stat st;
    int result = (fd < 0 ? stat(dir, &st) : fstat(fd, &st));

    if(result == 0)
    {
        mode_t dir_mode = st.st_mode;

        /* Check whether DIR is a directory. If FD is nonnegative, this
           check avoids changing the ownership and mode bits of the
           wrong file in many cases. This doesn't fix all the race
           conditions, but it is better than nothing. */
        if(! S_ISDIR(dir_mode))
        {
            errno = ENOTDIR;
            result = -1;
        }
        else
        {
            /* If at least one of the S_IXUGO bits are set, chown might
               clear the S_ISUID and S_ISGID bits. Keep track of any
               file mode bits whose values are indeterminate due to this
               issue. */
            mode_t indeterminate = 0;

            /* On some systems, chown clears S_ISUID and S_ISGID, so do
               chown before chmod. On older System V hosts, ordinary
               users can give their files away via chown; don't worry
               about that here, since users shouldn't do that */

            if((owner != (uid_t) -1 && owner != st.st_uid)
                || (group != (gid_t) -1 && group != st.st_gid))
            {
                result = (fd >= 0
                          ? fchown(fd, owner, group)
                          : mkdir_mode != (mode_t) -1
                          ? lchown(dir, owner, group)
                          : chown(dir, owner, group));

                /* Either the user cares about an indeterminate bit and
                   it'll be set properly by chmod below, or the user
                   doesn't care and it's OK to use the bit's pre-chown
                   value. So there's no need to re-stat DIR here. */
                if(result == 0 && (dir_mode & S_IXUGO))
                    indeterminate = dir_mode & (S_ISUID | S_ISGID);
            }

            /* If the file mode bits might not be right, use chmod to
               change them. Don't change bits the user doesn't care
               about. */
            if(result == 0 && (((dir_mode ^ mode) | indeterminate) & mode_bits))
            {
                mode_t chmod_mode =
                            mode | (dir_mode & CHMOD_MODE_BITS & ~mode_bits);
                result = (HAVE_FCHMOD && fd >= 0
                            ? fchmod(fd, chmod_mode)
                            : mkdir_mode != (mode_t) -1
                            ? lchmod(dir, chmod_mode)
                            : chmod(dir, chmod_mode));
            }
        }
    }

    if(fd >= 0)
    {
        if(result == 0)
            result = close(fd);
        else
        {
            int e = errno;
            close(fd);
            errno = e;
        }
    }
    return result;
}
