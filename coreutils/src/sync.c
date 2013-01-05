#include <config.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>  // sync(2)

#include "system.h"
#include "error.h"
#include "long-options.h"
#include "closeout.h"
#include "version.h"

#define PROGRAM_NAME "sync"
//#define AUTHORS proper_name("Jim Meyering")
#define AUTHORS "Jim Meyering"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("Usage: %s [OPTION]\n"), program_name);
        fputs(_("Force changed blocks to disk, update the super block.\n\n"),
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

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE, Version,
                        usage, AUTHORS, (char const*)NULL);

    if(getopt_long(argc, argv, "", NULL, NULL) != -1)
        usage(EXIT_FAILURE);

    if(optind < argc)
        error(0, 0, _("ignoring all arguments"));

    sync();
    exit(EXIT_SUCCESS);
}
