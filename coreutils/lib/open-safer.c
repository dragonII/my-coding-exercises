/* Invoke open, but avoid some glitches */

#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>

#include "config.h"
#include "fcntl-safer.h"
#include "unistd-safer.h"

int open_safer(char* file, int flags, ...)
{
    mode_t mode = 0;

    if(flags & O_CREAT)
    {
        va_list ap;
        va_start(ap, flags);

        /* We have to use PROMOTED_MODE_T instead of mode_t, otherwise GCC 4
           create crashing code when 'mode_t' is smaller than 'int'. */
        mode = va_arg(ap, PROMOTED_MODE_T);

        va_end(ap);
    }

    return fd_safer(open(file, flags, mode));
}
