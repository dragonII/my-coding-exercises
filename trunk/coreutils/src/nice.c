/* nice -- run a program with modified niceness */

#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <error.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "long-options.h"
#include "xstrtol.h"
#include "system.h"
#include "closeout.h"
#include "quote.h"


#define PROGRAM_NAME "nice"
#define AUTHORS "David MacKenzie"

#define GET_NICENESS() getpriority (PRIO_PROCESS, 0)

#ifndef NZERO
# define NZERO 20
#endif

static struct option longopts[] =
{
    {"adjustment", required_argument, NULL, 'n'},
    {NULL, 0, NULL, 0}
};

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("Usage: %s [OPTION] [COMMAND [ARG]...]\n"), program_name);
        printf(_("\
Run COMMAND with an adjusted niceness, which affects process scheduling.\n\
With no COMMAND, print the current niceness. Nicenesses range from\n\
%d (most favorable scheduling) to %d (least favorable).\n\
\n\
  -n, --adjustment=N    add integer N to the niceness (default 10)\n\
"),
        - NZERO, NZERO - 1);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        printf(USAGE_BUILTIN_WARNING, PROGRAM_NAME);
        emit_ancillary_info();
    }
    exit(status);
}

static bool perm_related_errno(int err)
{
    return err == EACCES || err == EPERM;
}

int main(int argc, char** argv)
{
    int current_niceness;
    int adjustment = 10;
    char* adjustment_given = NULL;
    bool ok;
    int i;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    initialize_exit_failure(EXIT_CANCELED);
    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char*)NULL);

    for(i = 1; i < argc; /* empty */)
    {
        char* s = argv[i];

        if(s[0] == '-' && ISDIGIT(s[1 + (s[1] == '-' || s[1] == '+')]))
        {
            adjustment_given = s + 1;
            ++i;
        }
        else
        {
            int optc;
            int fake_argc = argc - (i - 1);
            char** fake_argv = argv + (i - 1);

            /* Ensure that any getopt diagnostics use the right name */
            fake_argv[0] = argv[0];

            /* Initialize getopt_long's internal state */
            optind = 0;

            optc = getopt_long(fake_argc, fake_argv, "+n:", longopts, NULL);
            i += optind - 1;

            if(optc == '?')
                usage(EXIT_CANCELED);
            else if(optc == 'n')
                adjustment_given = optarg;
            else /* optc == -1 */
                break;
        }
    }

    if(adjustment_given)
    {
        /* If the requested adjustment is outside the valid range,
           silently bring ti to just within range, this mimics what
           "setpriority" and "nice" do */
        enum { MIN_ADJUSTMENT = 1 - 2 * NZERO, MAX_ADJUSTMENT = 2 * NZERO - 1 };
        long int tmp;
        if(LONGINT_OVERFLOW < xstrtol(adjustment_given, NULL, 10, &tmp, ""))
            error(EXIT_CANCELED, 0, _("invalid adjustment %s"),
                    quote(adjustment_given));
        adjustment = MAX(MIN_ADJUSTMENT, MIN(tmp, MAX_ADJUSTMENT));
    }

    if(argc == i)
    {
        if(adjustment_given)
        {
            error(0, 0, _("a command must be given with an adjustment"));
            usage(EXIT_CANCELED);
        }
        /* No command given; print the niceness */
        errno = 0;
        current_niceness = GET_NICENESS();
        if(current_niceness == -1 && errno != 0)
            error(EXIT_CANCELED, errno, _("cannot get niceness"));
        printf("%d\n", current_niceness);
        exit(EXIT_SUCCESS);
    }

    errno = 0;
    current_niceness = GET_NICENESS();
    if(current_niceness == -1 && errno != 0)
        error(EXIT_CANCELED, errno, _("cannot get niceness"));
    ok = (setpriority(PRIO_PROCESS, 0, current_niceness + adjustment) == 0);
    if(!ok)
    {
        error(perm_related_errno(errno) ? 0
                : EXIT_CANCELED, errno, _("cannot set niceness"));
        /* error() flushes stderr, but does not check for write failure.
           Normally, we would catch this via our atexit() hook of
           close_stdout, but execvp() gets in the way. If stderr
           encountered a write failure, there is no need to try calling
           error() again. */
        if(ferror(stderr))
            exit(EXIT_CANCELED);
    }

    execvp(argv[i], &argv[i]);

    {
        int exit_status = (errno == ENOENT ? EXIT_ENOENT : EXIT_CANNOT_INVOKE);
        error(0, errno, "%s", argv[i]);
        exit(exit_status);
    }
}
