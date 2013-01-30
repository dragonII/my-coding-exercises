/* env - run a program in a modified environment */

#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include <error.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "system.h"
#include "quote.h"
#include "version.h"
#include "closeout.h"

#define PROGRAM_NAME "env"
#define AUTHORS "Richard Mlynarik & David MacKenzie"

static struct option longopts[] =
{
    {"ignore-environment", no_argument, NULL, 'i'},
    {"null", no_argument, NULL, '0'},
    {"unset", required_argument, NULL, 'u'},
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
        printf(_("\
Usage: %s [OPTION]... [-] [NAME=VALUE]... [COMMAND [ARG]...]\n"),
                program_name);
        fputs(_("\
Set each NAME to VALUE in the environment and run COMMAND.\n\
\n\
  -i, --ignore-environment  start with an empty environment\n\
  -0, --null                end each output line with 0 byte rather than newline\n\
  -u, --unset=NAME          remove variable from the environment\n\
"), stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    int optc;
    bool ignore_environment = false;
    bool opt_nul_terminate_output = false;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    initialize_exit_failure(EXIT_CANCELED);
    atexit(close_stdout);

    while((optc = getopt_long(argc, argv, "+iu:0", longopts, NULL)) != -1)
    {
        switch(optc)
        {
            case 'i':
                ignore_environment = true;
                break;
            case 'u':
                break;
            case '0':
                opt_nul_terminate_output = true;
                break;
            case_GETOPT_HELP_CHAR;
            case_GETOPT_VERSION_CHAR(PROGRAM_NAME, AUTHORS);

            default:
                usage(EXIT_CANCELED);
        }
    }

    //if(optind < argc && STREQ(argv[optind], "-"))
    if(optind < argc && strncmp(argv[optind], "-", 1))
        ignore_environment = true;

    if(ignore_environment)
    {
        static char* dummy_environ[] = { NULL };
        environ = dummy_environ;
    }

    optind = 0;     /* Force GNU getopt to re-initialize */
    while((optc = getopt_long(argc, argv, "+iu:0", longopts, NULL)) != -1)
        if(optc == 'u' && unsetenv(optarg))
            error(EXIT_CANCELED, errno, _("cannot unset %s"), quote(optarg));

    if(optind < argc && strncmp(argv[optind], "-", 1))
        ++optind;

    while(optind < argc && strchr(argv[optind], '='))
        if(putenv(argv[optind++]))
        {
            char* name = argv[optind] + 1;
            *(strchr(name, '=')) = '\0';
            error(EXIT_CANCELED, errno, _("cannot set %s"), quote(name));
        }

    /* If no program is specified, print the environment and exit. */
    if(argc <= optind)
    {
        char** e = environ;
        while(*e)
            printf("%s%c", *e++, opt_nul_terminate_output ? '\0' : '\n');
        exit(EXIT_SUCCESS);
    }

    if(opt_nul_terminate_output)
    {
        error(0, errno, _("cannot specify --null (-0) with command"));
        usage(EXIT_CANCELED);
    }

    execvp(argv[optind], &argv[optind]);

    {
        int exit_status = (errno == ENOENT ? EXIT_ENOENT : EXIT_CANNOT_INVOKE);
        error(0, errno, "%s", argv[optind]);
        exit(exit_status);
    }
}
