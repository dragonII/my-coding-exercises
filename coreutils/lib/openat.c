/* provide a replacement openat function */

#include "openat.h"
#include "openat-priv.h"

/* Return true if our openat implementation must resort to
   using save_cwd and restore_cwd */
bool
openat_needs_fchdir(void)
{
    bool needs_fchdir = true;
    int fd = open("/", O_RDONLY);

    if(fd >= 0)
    {
        char buf[OPENAT_BUFFER_SIZE];
        char* proc_file = openat_proc_name(buf, fd, ".");
        if(proc_file)
        {
            needs_fchdir = false;
            if(proc_file != buf)
                free(proc_file);
        }
        close(fd);
    }

    return needs_fchdir;
}
