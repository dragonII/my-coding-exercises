/* Remove directory entries */

#ifndef REMOVE_H
#define REMOVE_H

#include <stdlib.h>
#include <stdbool.h>

#include "dev-ino.h"

enum rm_interactive
{
    /* Start with any number larger than 1, so that any legacy tests
       against values of 0 or 1 will fail */
    RMI_ALWAYS = 3,
    RMI_SOMETIMES,
    RMI_NEVER
};


struct rm_options
{
    /* If true, ignore nonexistent files */
    bool ignore_missing_files;

    /* If true, query the user about whether to remove each file */
    enum rm_interactive interactive;

    /* If true, do not traverse into (or remove) any directory that is
       on a file system (i.e., that has a different device number) other
       than that of the corresponding command line argument. Note that
       even without this option, rm will fail in the end, due to its
       probable inability to remove the mount point. But there, the
       diagnostic comes too late -- after removing all contents */
    bool one_file_system;

    /* If true, recursively remove directories */
    bool recursive;

    /* Pointer to the device and inode numbers of `/', when --recursive
       and preserving `/'. Otherwise NULL */
    struct dev_ino* root_dev_ino;

    /* If nonzero, stdin is a tty */
    bool stdin_tty;

    /* If true, display the name of each file removed */
    bool verbose;

    /* If true, treat the failure by the rm function to restore the
       current working directory as a fatal error. I.e., if this field
       is true and the rm function cannot restore cwd, it must exit with
       a nonzero status. Some applications require that the rm function
       restore cwd (e.g., mv) and some others do not (e.g., rm, 
       in many cases). */
    bool require_restore_cwd;
};

enum RM_status
{
    /* These must be listed in order of increasing seriousness */
    RM_OK = 2,
    RM_USER_DECLINED,
    RM_ERROR,
    RM_NONEMPTY_DIR
};

#define VALID_STATUS(S)     \
    ((S) == RM_OK || (S) == RM_USER_DECLINED || (S) == RM_ERROR)

#define UPDATE_STATUS(S, New_value)     \
    do                                  \
    {                                   \
        if((New_value) == RM_ERROR      \
            || ((New_value) == RM_USER_DECLINED && (S) == RM_OK))   \
            (S) = (New_value);          \
    } while(0)

enum RM_status
rm(char** file, struct rm_options* x);


#endif
