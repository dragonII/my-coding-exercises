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


#endif
