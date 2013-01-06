/* yes - output a string repeatedly until killed */

#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>

#include "system.h"
#include "closeout.h"
#include "long-options.h"
#include "version.h"
#include "error.h"

#define PROGRAM_NAME "yes"
#define AUTHORS "David MacKenzie"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                        program_name);
    else
    {
        printf(_("\
Usage: %s [STRING]...\n\
   or: %s OPTION\n\
"),
                program_name, program_name);
        fputs(_("\
Repeatedly output a line with all specified STRING(s), or `y'.\n\
\n\
"), stdout);
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
    bindtextdomain(PACKAGE, LOCALDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char *)NULL);
    if(getopt_long(argc, argv, "+", NULL, NULL) != -1)
        usage(EXIT_FAILURE);

    if(argc <= optind)
    {
        optind = argc;
        argv[argc++] = bad_cast("y");
    }

    for(;;)
    {
        int i;
        for(i = optind; i < argc; i++)
            if(fputs(argv[i], stdout) == EOF
                || putchar(i == argc - 1 ? '\n' : ' ') == EOF)
            {
                error(0, errno, _("standard output"));
                exit(EXIT_FAILURE);
            }
    }
}
