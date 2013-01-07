/* dirname -- strip suffix from file name */

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <stdlib.h>

#include "long-options.h"
#include "system.h"
#include "quote.h"
#include "closeout.h"
#include "version.h"
#include "dirname-lgpl.h"

#define PROGRAM_NAME "dirname"
#define AUTHORS "Jim Meyering and David MacKenzie"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("\
Usage: %s NAME\n\
   or: %s OPTION\n\
"),
                program_name, program_name);
        fputs(_("\
Print NAME with its trailing /component removed; if NAME contains no /'s, \n\
output `.' (meaning the current directory).\n\
\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        printf(_("\
\n\
Examples:\n\
   %s /usr/bin/sort   Output \"/usr/bin\".\n\
   %s stdio.h         Output \".\".\n\
"),
                program_name, program_name);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    static char dot = '.';
    char *result;
    size_t len;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char*)NULL);
    if(getopt_long(argc, argv, "+", NULL, NULL) != -1)
        usage(EXIT_FAILURE);

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

    result = argv[optind];
    len = dir_len(result);

    if(!len)
    {
        result = &dot;
        len = 1;
    }

    fwrite(result, 1, len, stdout);
    putchar('\n');

    exit(EXIT_SUCCESS);
}
