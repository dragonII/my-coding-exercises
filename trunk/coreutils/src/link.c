/* link utility for GNU
   Implementation overview:

   Simple call the system `link' function */

#include <stdio.h>
#include <getopt.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "long-options.h"
#include "quote.h"
#include "system.h"
#include "version.h"
#include "closeout.h"

#define PROGRAM_NAME "link"
#define AUTHORS "Michael Stone"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help for more information.\n"),
                program_name);
    else
    {
        printf(_("\
Usage: %s FILE1 FILE2\n\
   or: %s OPTION\n"), program_name, program_name);
        fputs(_("Call the link function to create a link named FILE2\
 to an existing FILE1.\n\n"),
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
    if(getopt_long(argc, argv, "", NULL, NULL) != -1)
        usage(EXIT_FAILURE);

    if(argc < optind + 2)
    {
        if(argc < optind + 1)
            error(0, 0, _("missing operand"));
        else
            error(0, 0, _("missing operand after %s"), quote(argv[optind]));
        usage(EXIT_FAILURE);
    }

    if(optind + 2 < argc)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind + 2]));
        usage(EXIT_FAILURE);
    }

    if(link(argv[optind], argv[optind + 1]) != 0)
        error(EXIT_FAILURE, errno, _("cannot create link %s to %s"),
                quote(argv[optind + 1]), quote(argv[optind]));
    exit(EXIT_SUCCESS);
}
