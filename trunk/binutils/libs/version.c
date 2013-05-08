/* version.c -- binutils version information */

#include <stdio.h>
#include <bfd.h>

#include "include/bucomm.h"
#include "include/bfdver.h"

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

