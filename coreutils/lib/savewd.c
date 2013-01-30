/* Save and restore the working directory, possibly using a child process */

#include "config.h"
#include "savewd.h"
#include "fcntl-safer.h"
#include "dirname.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

/* Initialize a saved working directory object */
static inline void
savewd_init(struct savewd* wd)
{
    wd->state = INITIAL_STATE;
}

void savewd_finish(struct savewd* wd)
{
    switch(wd->state)
    {
        case INITIAL_STATE:
        case ERROR_STATE:
            break;

        case FD_STATE:
        case FD_POST_CHDIR_STATE:
            close(wd->val.fd);
            break;

        case FORKING_STATE:
            assert(wd->val.child < 0);
            break;

        default:
            assert(false);
    }

    wd->state = FINAL_STATE;
}

/* Save the working directory into *WD, if it hasn't been saved
   already. Return true if a child has been forked to do the real
   work */
static bool
savewd_save(struct savewd* wd)
{
    switch(wd->state)
    {
        case INITIAL_STATE:
            /* Save the working directory, or perpare to fall back if possible */
            {
                int fd = open_safer(".", O_RDONLY);
                if(fd >= 0)
                {
                    wd->state = FD_STATE;
                    wd->val.fd = fd;
                    break;
                }
                if(errno != EACCES && errno != ESTALE)
                {
                    wd->state = ERROR_STATE;
                    wd->val.errnum = errno;
                    break;
                }
            }
            wd->state = FORKING_STATE;
            wd->val.child = -1;
            /* Fall through */
        case FORKING_STATE:
            if(wd->val.child < 0)
            {
                /* "Save" the initial working directory by forking a new
                   subprocess that will attempt all the work from the chdir
                   until the next savewd_restore */
                wd->val.child = fork();
                if(wd->val.child != 0)
                {
                    if(wd->val.child > 0)
                        return true;
                    wd->state = ERROR_STATE;
                    wd->val.errnum = errno;
                }
            }
            break;

        case FD_STATE:
        case FD_POST_CHDIR_STATE:
        case ERROR_STATE:
        case FINAL_STATE:
            break;

        default:
            assert(false);
    }
    return false;
}

int savewd_chdir(struct savewd* wd, char* dir, int options,
                 int open_result[2])
{
    int fd = -1;
    int result = 0;

    /* Open the directory if requested, or if avoiding a race condition
       is requested and possible. */
    if(open_result 
        || (options & (HAVE_WORKING_O_NOFOLLOW ? SAVEWD_CHDIR_NOFOLLOW : 0)))
    {
        fd = open(dir,
                   (O_RDONLY | O_DIRECTORY | O_NOCTTY | O_NONBLOCK
                    | (options & SAVEWD_CHDIR_NOFOLLOW ? O_NOFOLLOW : 0)));
        if(open_result)
        {
            open_result[0] = fd;
            open_result[1] = errno;
        }

        if(fd < 0 && (errno != EACCES || (options & SAVEWD_CHDIR_READABLE)))
            result = -1;
    }

    if(result == 0 && !(fd >= 0 && options & SAVEWD_CHDIR_SKIP_READABLE))
    {
        if(savewd_save(wd))
        {
            open_result = NULL;
            result = -2;
        }
        else
        {
            result = (fd < 0 ? chdir(dir) : fchdir(fd));
            if(result == 0)
                switch(wd->state)
                {
                    case FD_STATE:
                        wd->state = FD_POST_CHDIR_STATE;
                        break;
                    case ERROR_STATE:
                    case FD_POST_CHDIR_STATE:
                    case FINAL_STATE:
                        break;
                    case FORKING_STATE:
                        assert(wd->val.child == 0);
                        break;
                    default:
                        assert(false);
                }
        }
    }
    if(fd >= 0 && !open_result)
    {
        int e = errno;
        close(fd);
        errno = e;
    }

    return result;
}


int savewd_restore(struct savewd* wd, int status)
{
    switch(wd->state)
    {
        case INITIAL_STATE:
        case FD_STATE:
            /* The working directory is the desired directory, so there's no
               work to do */
            break;

        case FD_POST_CHDIR_STATE:
            /* Restore the working directory using fchdir */
            if(fchdir(wd->val.fd) == 0)
            {
                wd->state = FD_STATE;
                break;
            }
            else
            {
                int chdir_errno = errno;
                close(wd->val.fd);
                wd->state = ERROR_STATE;
                wd->val.errnum = chdir_errno;
            }
            /* Fall throught */
        case ERROR_STATE:
            /* Report an error if asked to restore the working directory */
            errno = wd->val.errnum;
            return -1;

        case FORKING_STATE:
            /* "Restore" the working directory by waiting for the subprocess
               to finish */
            {
                pid_t child = wd->val.child;
                if(child == 0)
                    _exit(status);
                if(child > 0)
                {
                    int child_status;
                    while(waitpid(child, &child_status, 0) < 0)
                        assert(errno == EINTR);
                    wd->val.child = -1;
                    if(! WIFEXITED(child_status))
                        raise(WTERMSIG(child_status));
                    return WEXITSTATUS(child_status);
                }
            }
            break;

        default:
            assert(false);
    }
    return 0;
}

/* Return true if the actual work is currently being done by a
   subprocess.

   A true return means that the caller and the subprocess should
   resynchronize later with savewd_restore, using only their own
   memory to decide when to resynchronize; they should not consult the
   file system to decide, because that might lead to race conditions.
   This is why savewd_chdir is broken out into another function;
   savewd_chdir's callers _can_ inspect the file system to decide
   whether to call savewd_chdir */
static inline bool
savewd_delegating(struct savewd* wd)
{
    return wd->state == FORKING_STATE && wd->val.child > 0;
}

int savewd_process_files(int n_files, char** file,
                         int (*act)(char*, struct savewd*, void*),
                         void* options)
{
    int i = 0;
    int last_relative;
    int exit_status = EXIT_SUCCESS;
    struct savewd wd;
    savewd_init(&wd);

    for(last_relative = n_files - 1; last_relative >= 0; last_relative--)
        if(! IS_ABSOLUTE_FILE_NAME(file[last_relative]))
            break;

    for(; i < last_relative; i++)
    {
        if(!savewd_delegating(&wd))
        {
            int s = act(file[i], &wd, options);
            if(exit_status < s)
                exit_status = s;
        }

        if(! IS_ABSOLUTE_FILE_NAME(file[i + 1]))
        {
            int r = savewd_restore(&wd, exit_status);
            if(exit_status < r)
                exit_status = r;
        }
    }

    savewd_finish(&wd);

    for(; i < n_files; i++)
    {
        int s = act(file[i], &wd, options);
        if(exit_status < s)
            exit_status = s;
    }

    return exit_status;
}
