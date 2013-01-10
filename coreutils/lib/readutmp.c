/* GNU's read utmp module */


#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "readutmp.h"
#include "xalloc.h"


/* Copy UT->ut_name into storage obtained from malloc. Then remove any
   trailing spaces from the copy, NULL terminate it, and return the copy */
char* extract_trimmed_name(const STRUCT_UTMP* ut)
{
    char* p, *trimmed_name;

    trimmed_name = xmalloc(sizeof(UT_USER(ut)) + 1);
    strncpy(trimmed_name, UT_USER(ut), sizeof(UT_USER(ut)));
    /* Append a trailing NUL. Some systems pad names shorter than the
       maximum with spaces, others pad with NULs. Remove any trailing
       spaces. */
    trimmed_name[sizeof(UT_USER(ut))] = '\0';
    for(p = trimmed_name + strlen(trimmed_name);
            trimmed_name < p && p[-1] == ' ';
            *--p = '\0')
        continue;

    return trimmed_name;
}


/* Is the utmp entry U desired by the user who asked for OPTIONS? */
bool desirable_utmp_entry(STRUCT_UTMP* u, int options)
{
    bool user_proc = IS_USER_PROCESS(u);
    if((options & READ_UTMP_USER_PROCESS) && !user_proc)
        return false;
    if((options & READ_UTMP_CHECK_PIDS)
        && user_proc
        && (UT_PID(u) <= 0
            || (kill (UT_PID(u), 0) < 0 && errno == ESRCH)))
        return false;
    return true;
}


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
    int count = 1;

    /* Ignore the return value for now.
       Solaris's utmpname returns 1 upon success -- which is contrary
       to what the GNU libc version does. In additional, older GNU libc
       versions are actually void. */
    UTMP_NAME_FUNCTION(file);

    SET_UTMP_ENT();

    while((u = GET_UTMP_ENT()) != NULL)
    {
        printf("count = %d\n", count);
        if(desirable_utmp_entry(u, options))
        {
            if(n_read == n_alloc)
                utmp = x2nrealloc(utmp, &n_alloc, sizeof *utmp);

            utmp[n_read++] = *u;
        }
        count++;
    }

    END_UTMP_ENT();

    *n_entries = n_read;
    *utmp_buf = utmp;

    return 0;
}
