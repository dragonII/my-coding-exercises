/* hostname - set or print the name of current host system.
   Currently, no sethostname implemented, only get */

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>

#include "system.h"
#include "long-options.h"
#include "quote.h"
#include "version.h"
#include "closeout.h"
#include "xgethostname.h"

#define PROGRAM_NAME "hostname"
#define AUTHORS "Jim Meyering"

#if !defined HAVE_SETHOMENAME && HAVE_SYSINFO && \
     defined HAVE_SYS_SYSTEMINFO_H
#include <sys/sysinfo.h>

static int sethostname(char* name, size_t namelen)
{
    /* Using sysinfo() is the SVR4 mechanism to set a hostname */
    return (sysinfo(SI_SET_HOSTNAME, name, namelen) < 0 ? -1 : 0)
}

#define HAVE_SETHOMENAME 1 /* now we have it */
#endif

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("\
Usage: %s [NAME]\n\
   or: %s OPTION\n\
Print or set the hostname of the current system.\n\
\n\
"),
                program_name, program_name);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    char* hostname;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);
    
    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char*)NULL);
    if(getopt_long(argc, argv, "", NULL, NULL) != -1)
        usage(EXIT_FAILURE);

    if(argc == optind + 1)
    {
#ifdef HAVE_SETHOMENAME
        /* Set hostname to operand */
        char* name = argv[optind];
        if(sethostname(name, strlen(name)) != 0)
            error(EXIT_FAILURE, errno, _("cannot set name to %s"), quote(name));
#else
        error(EXIT_FAILURE, 0,
                _("cannot set hostname; this system lacks the functionality"));
#endif
    }

    if(argc <= optind)
    {
        hostname = xgethostname();
        if(hostname == NULL)
            error(EXIT_FAILURE, errno, _("cannot determine hostname"));
        printf("%s\n", hostname);
    }

    if(optind + 1 < argc)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind + 1]));
        usage(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
