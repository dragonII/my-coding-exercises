/* unlink utility for GNU 
   Implementation overview:

        Simple call the system `unlink' function */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>  // unlink(2)

#include "gettext.h"
#include "config.h"
#include "system.h"
#include "closeout.h"
#include "long-options.h"
#include "version.h"
#include "quote.h"

#define PROGRAM_NAME "unlink"
#define AUTHORS "Michael Stone"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("\
Usage: %s FILE\n\
   or: %s OPTION\n"), program_name, program_name);
        fputs(_("Call the unlink function to remove the specified FILE.\n\n"),
                stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char*)NULL);
    int rc = getopt_long(argc, argv, "", NULL, NULL);
    if(rc != -1)
        usage(EXIT_FAILURE);

    printf("argc = %d, optind = %d\n", argc, optind);
    if(argc < optind + 1)
    {
        error(0, 0, _("missing operand"));
        usage(EXIT_FAILURE);
    }
    if(optind + 1 < argc)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind + 1]));
        usage(EXIT_FAILURE);
    }
    if(unlink(argv[optind]) != 0)
        error(EXIT_FAILURE, errno, _("cannot unlink %s"), quote(argv[optind]));

    exit(EXIT_SUCCESS);
}
