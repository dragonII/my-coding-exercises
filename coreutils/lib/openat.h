/* provide a replacement openat function */

#ifndef _GL_HEADER_OPENAT
#define _GL_HEADER_OPENAT

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef OPENAT_BUFFER_SIZE
#define OPENAT_BUFFER_SIZE 512
#endif

/* Using these function names makes application code
   slightly more readable than it would be with
   fchownat (..., 0) or fchownat (..., AT_SYMLINK_NOFOLLOW). */
static inline int
chownat(int fd, char* file, uid_t owner, gid_t group)
{
    return fchownat(fd, file, owner, group, 0);
}

static inline int
lchownat(int fd, char* file, uid_t owner, gid_t group)
{
    return fchownat(fd, file, owner, group, AT_SYMLINK_NOFOLLOW);
}

static inline int
lstatat(int fd, char* name, struct stat* st)
{
    return fstatat(fd, name, st, AT_SYMLINK_NOFOLLOW);
}

bool openat_needs_fchdir(void);



#endif
