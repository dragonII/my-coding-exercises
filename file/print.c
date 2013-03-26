/* print.c - debugging printout routines */

#include "file.h"

#include <stdarg.h>
#include <stdio.h>

void file_magwarn(struct magic_set* ms, const char* f, ...)
{
    va_list va;

    /* because we use stdout for most, stderr here */
    (void)fflush(stdout);

    if(ms->file)
        (void)fprintf(stderr, "%s, %lu: ", ms->file,
                (unsigned long)ms->line);
    (void)fprintf(stderr, "Warning: ");
    va_start(va, f);
    (void)vfprintf(stderr, f, va);
    va_end(va);
    (void)fputc('\n', stderr);
}
