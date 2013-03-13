/* euidaccess-stat -- check if effective user id can access lstat'd file
   This function is probably useful only for choosing whether to issue
   a prompt in an implementation of POSIX-specified rm. */

#include "euidaccess-stat.h"

#include <unistd.h>

/* Return true if the current user has permission of type MODE
   on the file from which stat buffer *ST was obtained, ignoring
   ACLs, attributes, `read-only'ness, etc...
   Otherwise, return false.

   Like the reentrant version of euidaccess, but starting with
   a stat buffer rather than a file name. Hence, this function
   never calls access or accessx, and doesn't take into account
   whether the file has ACLs or other attributes, or resides on
   a read-only file system. */
bool
euidaccess_stat(struct stat* st, int mode)
{
    uid_t euid;
    unsigned int granted;

    /* Convert the mode to traditional form, clearing any bogus bits */
    if(R_OK == 4 && W_OK == 2 && X_OK == 1 && F_OK == 0)
        mode &= 7;
    else
        mode = ((mode & R_OK ? 4 : 0)
                + (mode & W_OK ? 2 : 0)
                + (mode & X_OK ? 1 : 0));
    if(mode == 0)
        return true;        /* The file exists */

    euid = geteuid();

    /* The super-user can read and write any file, and execute any file
       that anyone can execute */
    if(euid == 0 && ((mode & X_OK) == 0
                     || (st->st_mode & (S_IXUSR | S_IXGRP |S_IXOTH))))
        return true;

    /* Convert the file's permission bits to traditional form */
    if(    S_IRUSR == (4 << 6)
        && S_IWUSR == (2 << 6)
        && S_IXUSR == (1 << 6)
        && S_IRGRP == (4 << 3)
        && S_IWGRP == (2 << 3)
        && S_IXGRP == (1 << 3)
        && S_IROTH == (4 << 0)
        && S_IWOTH == (2 << 0)
        && S_IXOTH == (1 << 0))
        granted = st->st_mode;
    else
        granted = (  (st->st_mode & S_IRUSR ? 4 << 6 : 0)
                   + (st->st_mode & S_IWUSR ? 2 << 6 : 0)
                   + (st->st_mode & S_IXUSR ? 1 << 6 : 0)
                   + (st->st_mode & S_IRGRP ? 4 << 3 : 0)
                   + (st->st_mode & S_IWGRP ? 2 << 3 : 0)
                   + (st->st_mode & S_IXGRP ? 1 << 3 : 0)
                   + (st->st_mode & S_IROTH ? 4 << 0 : 0)
                   + (st->st_mode & S_IWOTH ? 2 << 0 : 0)
                   + (st->st_mode & S_IXOTH ? 1 << 0 : 0));

        if(euid == st->st_uid)
            granted >>= 6;
        else
        {
            gid_t egid = getegid();
            if(egid == st->st_gid || group_member(st->st_gid))
                granted >>= 3;
        }

        if((mode & ~granted) == 0)
            return true;

        return false;
}
