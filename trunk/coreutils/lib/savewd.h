/* Save and restore the working directory, possibly using a subprocess */

#ifndef SAVEWD_H
#define SAVEWD_H

#include <stdbool.h>
#include <sys/types.h>

/* A saved working directory. The member names and constants defined
   by this structure are private to the savewd module. */
struct savewd
{
    /* The state of this object */
    enum
    {
        /* This object has been created but does not yes represent
           the working directory */
        INITIAL_STATE,

        /* val.fd is the original working directory's file descriptor.
           It is still the working directory */
        FD_STATE,

        /* Like FD_STATE, but the working directory has changed, so
           restoring it will require a fchdir */
        FD_POST_CHDIR_STATE,

        /* Fork and let the subprocess do the work. val.child is 0 in a 
           child, negative in a childless parent, and the child process
           ID in a parent with a child. */
        FORKING_STATE,

        /* A serious problem argues against further efforts. val.errnum
           contains the error number (e.g., EIO). */
        ERROR_STATE,

        /* savewd_finish has been called, so the application no longer
           cares whether the working directory is saved, and there is no
           more work to do. */
        FINAL_STATE
    } state;

    /* The object's value */
    union
    {
        int fd;
        int errnum;
        pid_t child;
    } val;
};

/* Options for savewd_chdir */
enum
{
    /* Do not follow symbolic links, if supported */
    SAVEWD_CHDIR_NOFOLLOW = 1,

    /* The directory should be readable, so fail if it happens to be
       discovered that the directory is not readable. (Unreadable
       directories are not necessarily diagnosed, though). */
    SAVEWD_CHDIR_READABLE = 2,

    /* Do not chdir if the directory is readable; simply succeed
       without invoking chdir if the directory was opened. */
    SAVEWD_CHDIR_SKIP_READABLE = 4
};

/* Return WD's error number, or 0 if WD is not in an error state */
static inline int savewd_errno(struct savewd* wd)
{
    return (wd->state == ERROR_STATE ? wd->val.errnum : 0);
}


int savewd_chdir(struct savewd* wd, char* dir, int options,
                    int open_result[2]);

int savewd_process_files(int n_files, char** file,
                         int (*act)(char*, struct savewd*, void*),
                         void* options);
int savewd_restore(struct savewd* wd, int status);

#endif
