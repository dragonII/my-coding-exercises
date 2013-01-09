/* GNU's read utmp module */


#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "readutmp.h"
#include "xalloc.h"

/* Read the utmp entries corresponding to file FILE into freshly-
   malloc'd storage, set *UTMP_BUF to that pointer, set *N_ENTRIES to
   the number of entries, and return zero. If there is any error,
   return -1, setting errno, and don't modify the parameters.
   If OPTIONS & READ_UTMP_CHECK_PIDS is nonzero, omit entries whose
   process-IDs do not currently exist. */
int read_utmp(const char* file, size_t *n_entries, STRUCT_UTMP **utmp_buf,
                int options)
{
    size_t n_read = 0;
    size_t n_alloc = 0;
    STRUCT_UTMP* utmp = NULL;
    STRUCT_UTMP* u;

    /* Ignore the return value for now.
       Solaris's utmpname returns 1 upon success -- which is contrary
       to what the GNU libc version does. In additional, older GNU libc
       versions are actually void. */
    UTMP_NAME_FUNCTION(file);

    SET_UTMP_ENT();

    while((u = GET_UTMP_ENT()) != NULL)
        if(desirable_utmp_entry(u, options))
        {
            if(n_read == n_alloc)
                utmp = x2nrealloc(utmp, &n_alloc, sizeof *utmp);

            utmp[n_read++] = *u;
        }

    END_UTMP_ENT();

    *n_entries = n_read;
    *utmp_buf = utmp;

    return 0;
}
