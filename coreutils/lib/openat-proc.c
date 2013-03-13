/* Create /proc/self/fd-related names for subfiles of open directories. */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dirname.h"
#include "xalloc.h"
#include "openat.h"
#include "same-inode.h"
#include "openat-priv.h"
#include "intprops.h"


/* The results of open() in this file are not used with fchdir,
   and we do not leak fds to any single-threaded code that could use stdio,
   therefore save some unnecessary work in fchdir.c */
#define PROC_SELF_FD_FORMAT "/proc/self/fd/%d/%s"

#define PROC_SELF_FD_NAME_SIZE_BOUND(len)       \
        (sizeof PROC_SELF_FD_FORMAT - sizeof "%d%s" \
        + INT_STRLEN_BOUND(int) + (len) + 1)

/* Set BUF to the expansion of PROC_SELF_FD_FORMAT, using FD and FILE
   respectively for %d and %s. If successful, return BUF if the
   result fits in BUF, dynamically allocated memory otherwise. But
   return NULL if /proc is not writable */
char* 
openat_proc_name(char buf[OPENAT_BUFFER_SIZE], int fd, char* file)
{
    static int proc_status = 0;

    /* Make sure the caller get ENOENT when appropriate */
    if(! *file)
    {
        buf[0] = '\0';
        return buf;
    }

    if(!proc_status)
    {
        /* Set PROC_STATUS to a positive value if /proc/self/fd is
           reliable, and a negative value otherwise. Solaris 10
           /proc/self/fd mishandles "..", and any file name might expand
           to ".." after symbolic link expansion, so avoid /proc/self/fd
           if it mishandles "..". Solaris 10 has openat, but this
           problem is exhibited on code that built on Solaris 8 and
           running on Solaris 10. */

        int proc_self_fd = open("/proc/self/fd", O_RDONLY);
        if(proc_self_fd < 0)
            proc_status = -1;
        else
        {
            struct stat proc_self_fd_dotdot_st;
            struct stat proc_self_st;
            char dotdot_buf[PROC_SELF_FD_NAME_SIZE_BOUND (sizeof ".." - 1)];
            sprintf(dotdot_buf, PROC_SELF_FD_FORMAT, proc_self_fd, "..");
            proc_status =
                ((stat(dotdot_buf, &proc_self_fd_dotdot_st) == 0
                  && stat("/proc/self", &proc_self_st) == 0
                  && SAME_INODE(proc_self_fd_dotdot_st, proc_self_st))
                 ? 1 : -1);
            close(proc_self_fd);
        }
    }

    if(proc_status < 0)
        return NULL;
    else
    {
        size_t bufsize = PROC_SELF_FD_NAME_SIZE_BOUND(strlen(file));
        char* result = (bufsize < OPENAT_BUFFER_SIZE ? buf : xmalloc(bufsize));
        sprintf(result, PROC_SELF_FD_FORMAT, fd, file);
        return result;
    }
}
