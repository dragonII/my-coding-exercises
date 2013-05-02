/* bucomm.c -- Bin Utils COmmon code.

   This file is part of GNU Binutils.

 */

#include "bucomm.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

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


void non_fatal (const char *format, ...)
{
    VA_OPEN(args, format);
    VA_FIXEDARG(args, const char *, format);

    report(format, args);
    VA_CLOSE(args);
}


/* Print the version number and copyright information, and exit. This
   implements the --version option for the various programs */
void print_version(const char *name)
{
    /* this output is intended to follow the GNUM standards document */
    /* xgettext:c-format */
    printf("GNU %s %s\n", name, BFD_VERSION_STRING);
    printf(_("Copyright 2005 Free Software Foundation, Inc.\n"));
    printf(_("\
This program is free software; you may redistribute it under the terms of\n\
the GNU General Public License. This program has absolutely no warranty.\n"));
    exit(0);
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
