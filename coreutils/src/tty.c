/* tty -- print the name of the terminal connected to standard input

   Displays "not a tty" if stdin is not a terminal.
   Displays nothing is -s option is given.
   Exit status 0 if stdin is a tty, 1 if not, 2 if usage error,
   3 if write error. */

#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <error.h>
#include <stdlib.h>
#include <unistd.h>

#include "system.h"
#include "quote.h"
#include "closeout.h"
#include "version.h"

/* Exit status */
enum
{
    TTY_FAILURE = 2,
    TTY_WRITE_ERROR = 3
};

#define PROGRAM_NAME "tty"
#define AUTHORS "David MacKenzie"

/* If true, return an exit status but produce no output */
static bool silent;

static struct option const longopts[] =
{
    {"silent", no_argument, NULL, 's'},
    {"quiet", no_argument, NULL, 's'},
    {GETOPT_HELP_OPTION_DECL},
    {GETOPT_VERSION_OPTION_DECL},
    {NULL, 0, NULL, 0}
};

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("Usage: %s [OPTION]...\n"), program_name);
        fputs(_("\
Print the file name of the terminal connected to standard input.\n\
\n\
  -s, --silent, --quiet     print nothing, only return an exit status.\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    char* tty;
    int optc;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    initialize_exit_failure(TTY_WRITE_ERROR);
    atexit(close_stdout);

    silent = false;

    while((optc = getopt_long(argc, argv, "s", longopts, NULL)) != -1)
    {
        switch(optc)
        {
            case 's':
                silent = true;
                break;
            case_GETOPT_HELP_CHAR;

            case_GETOPT_VERSION_CHAR(PROGRAM_NAME, AUTHORS);

            default:
                usage(TTY_FAILURE);
        }
    }

    if(optind < argc)
        error(0, 0, _("extra operand %s"), quote(argv[optind]));

    tty = ttyname(STDIN_FILENO);
    if(!silent)
    {
        if(tty)
            puts(tty);
        else
            puts(_("not a tty"));
    }
    
    exit(isatty(STDIN_FILENO) ? EXIT_SUCCESS : EXIT_FAILURE);
}
