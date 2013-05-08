/* bucomm.c -- Bin Utils COmmon code.

   This file is part of GNU Binutils.

 */

#include "include/bucomm.h"
#include "include/libiberty.h"
#include "include/ansidecl.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

char *program_name;


/* List the supported targets */
void list_supported_targets(const char *name, FILE *f)
{
    int t;
    const char **targ_names = bfd_target_list();

    if(name == NULL)
        fprintf(f, _("Supported targets: "));
    else
        fprintf(f, _("%s: supported targets:"), name);

    for(t = 0; targ_names[t] != NULL; t++)
        fprintf(f, " %s", targ_names[t]);
    fprintf(f, "\n");
    free(targ_names);
}


/* After FALSE return from bfd_check_format_matches with
   bfd_get_error() == bfd_error_file_ambiguously_recognized, print
   the possible matching targets */
void list_matching_formats(char **p)
{
    fprintf(stderr, _("%s: Matching formats:"), program_name);
    while(*p)
        fprintf(stderr, " %s", *p++);
    fputc('\n', stderr);
}

/* Set the default BFD target based on the configured target. Doing
   this permits the binutils to be configured for a particular target,
   and linked against a shared BFD library which was configured for a 
   different target */
void set_default_bfd_target(void)
{
    /* The macro TARGET is defined by bucomm.h */
    const char *target = TARGET;

    if(!bfd_set_default_target(target))
        fatal(_("can't set BFD default target to `%s': %s"),
            target, bfd_errmsg(bfd_get_error()));
}


void report(const char *format, va_list args)
{
    fprintf(stderr, "%s: ", program_name);
    vfprintf(stderr, format, args);
    putc('\n', stderr);
}


void fatal(const char *format, ...)
{
    VA_OPEN(args, format);
    VA_FIXEDARG(args, const char *, format);

    report(format, args);
    VA_CLOSE(args);
    xexit(1);
}


void non_fatal (const char *format, ...)
{
    VA_OPEN(args, format);
    VA_FIXEDARG(args, const char *, format);

    report(format, args);
    VA_CLOSE(args);
}


/* Returns the size of the named file. If the file does not
   exist, or if it is not a real file, then a suitable non-fatal
   error message is printed and zero is returned */
off_t get_file_size(const char *file_name)
{
    struct stat statbuf;
    
    if(stat(file_name, &statbuf) < 0)
    {
        if(errno == ENOENT)
            non_fatal(_("'%s': No such file"), file_name);
        else
            non_fatal(_("Warning: could not locate '%s'. reason: %s"),
                    file_name, strerror(errno));
    } else if(!S_ISREG(statbuf.st_mode))
        non_fatal(_("Warning: '%s' is not an ordinary file"), file_name);
    else
        return statbuf.st_size;

    return 0;
}


void bfd_nonfatal(const char *string)
{
    const char *errmsg = bfd_errmsg(bfd_get_error());

    if(string)
        fprintf(stderr, "%s: %s: %s\n", program_name, string, errmsg);
    else
        fprintf(stderr, "%s: %s\n", program_name, errmsg);
}
