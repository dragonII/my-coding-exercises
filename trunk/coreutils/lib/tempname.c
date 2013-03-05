/* tempname.c -- generate the name of a temporary file */

#include <sys/types.h>
#include <error.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <errno.h>
#ifndef __set_errno
# define __set_errno(Val) errno = (Val)
#endif

#include <assert.h>

#include <stdio.h>
#ifndef P_tmpdir
# define P_tmpdir "/tmp"
#endif
#ifndef TMP_MAX
# define TMP_MAX 238328
#endif
#ifndef __GT_FILE
# define __GT_FILE      0
# define __GT_DIR       1
# define __GT_NOCREATE  2
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define struct_stat64 struct stat
#define __mkdir mkdir
#define __open open
#define __lxstat64(version, file, buf) lstat(file, buf)

#include "randint.h"

static inline bool
check_x_suffix(char* s, size_t len)
{
    return len <= strspn(s, "X");
}

/* These are the characters used in temporary file names */
static char letters[] =
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* Generate a temporary file name based on TMPL. TMPL must match the
   rules for mk[s]temp (i.e. end in at least X_SUFFIX_LEN "X"s,
   possibly with a suffix).
   The name constructed does not exist at the time the call to
   this function. TMPL is overwritten with the result.

   KIND may be one of
   __GT_NOCREATE:       simply verify that the name does exist
                        at the time of the call
   __GT_FILE:           create the file using open(O_CREAT|O_EXCL)
                        and return a read-write fd. The file is mode 0600.
   __GT_DIR:            create a directory, which will be mode 0700.

   We use a clever algorithm to get hard-to-predict names */
int gen_tempname_len(char* tmpl, int suffixlen, int flags, int kind,
                        size_t x_suffix_len)
{
    size_t len;
    char* XXXXXX;
    unsigned int count;
    int fd = -1;
    int saved_errno = errno;
    struct_stat64 st;
    struct randint_source* rand_src;

    /* A lower bound on the number of temporary files to attempt to
       generate. The maximum total number of temporary file names that
       can exist for a given template is 62**6. It should never be 
       necessary to try all these combinations. Instead if a reasonable
       number of names is tried (we define reasonable as 62**3) fail to
       give the system adiministrator the chance to remove the problems.
       This value requires that X_SUFFIX_LEN be at least 3. */
#define ATTEMPTS_MIN (62 * 62 * 62)

    /* The number of times to attempt to generate a temporary file. To
       confirm to POSIX, this must be no smaller than TMP_MAX. */
#if ATTEMPTS_MIN < TMP_MAX
    unsigned int attempts = TMP_MAX;
#else
    unsigned int attempts = ATTEMPTS_MIN;
#endif

    len = strlen(tmpl);
    if(len < x_suffix_len + suffixlen
        || ! check_x_suffix(&tmpl[len - x_suffix_len - suffixlen],
                            x_suffix_len))
    {
        __set_errno(EINVAL);
        return -1;
    }

    /* This is where the Xs start */
    XXXXXX = &tmpl[len - x_suffix_len - suffixlen];

    /* Get some more or less random data */
    rand_src = randint_all_new(NULL, 8);
    if(!rand_src)
        return -1;

    for(count = 0; count < attempts; ++count)
    {
        size_t i;

        for(i = 0; i < x_suffix_len; i++)
            XXXXXX[i] = letters[randint_genmax(rand_src, sizeof letters - 2)];

        switch(kind)
        {
            case __GT_FILE:
                fd = __open(tmpl,
                            (flags & ~O_ACCMODE)
                            | O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
                break;
            case __GT_DIR:
                fd = __mkdir(tmpl, S_IRUSR | S_IWUSR |S_IXUSR);
                break;
            case __GT_NOCREATE:
                /* This case is backward from the other three. This function
                   succeeds if __xstat fails because the name does not exist.
                   Note the continue to bypass the common logic at the bottom
                   of the loop */
                if(__lxstat64(_STAT_VER, tmpl, &st) < 0)
                {
                    if(errno == ENOENT)
                    {
                        __set_errno(saved_errno);
                        fd = 0;
                        goto done;
                    }
                    else
                    {
                        /* Give up now */
                        fd = -1;
                        goto done;
                    }
                }
                continue;

            default:
                assert(! "invalid KIND in __gen_tempname");
                abort();
        }

        if(fd >= 0)
        {
            __set_errno(saved_errno);
            goto done;
        }
        else if(errno != EEXIST)
        {
            fd = -1;
            goto done;
        }
    }

    randint_all_free(rand_src);

    /* We got out of the loop because we ran out of combinations to try */
    __set_errno(EEXIST);
    return -1;

done:
    {
        int saved_errno1 = errno;
        randint_all_free(rand_src);
        __set_errno(saved_errno1);
    }
    return fd;
}
