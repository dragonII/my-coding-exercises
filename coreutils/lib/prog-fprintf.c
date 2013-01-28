/* prog-fprintf.c - common formating output functions and definitions */

#include <stdarg.h>
#include <sys/types.h>
#include <stdio.h>

#include "progname.h"
#include "prog-fprintf.h"
#include "progname.h"

/* Display program name followed by variable list.
   Used for e.g. verbose output */
void prog_fprintf(FILE* fp, const char* fmt, ...)
{
    va_list ap;
    fputs(program_name, fp);
    fputs(": ", fp);
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
    fputc('\n', fp);
    return;
}
