/* logname -- print user's login name */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include "progname.h"
#include "system.h"
#include "closeout.h"
#include "version.h"
#include "long-options.h"
#include "error.h"
#include "quote.h"

#define PROGRAM_NAME "logname"
#define AUTHORS ("unknown")

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                        program_name);
    else
    {
        printf(_("Usage: %s [OPTIONS]\n"), program_name);
        fputs(_("Print the name of the current user.\n\n"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    char* cp;
    int rc;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACAGE, LOCALEDIR);
    textdomain(PACAGE);

    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char const*)NULL);
    rc = getopt_long(argc, argv, "", NULL, NULL);
    printf("getopt_long return: %d\n", rc);
    if(rc != -1)
        usage(EXIT_FAILURE);

    printf("optind: %d\n", optind);
    if(optind < argc)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind]));
        usage(EXIT_FAILURE);
    }

    /* POSIX requires using getlogin (or equivalent code) */
    cp = getlogin();
    if(cp)
    {
        puts(cp);
        exit(EXIT_SUCCESS);
    }

    /* POSIX prohibits using a fallback technique */
    error(0, 0, _("no login name"));
    exit(EXIT_FAILURE);
}

