/* sleep -- delay for a specified amount of time */

#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include "system.h"
#include "long-options.h"
#include "quote.h"
#include "closeout.h"
#include "version.h"
#include "c-strtod.h"
#include "xstrtod.h"
#include "xnanosleep.h"

#define PROGRAM_NAME "sleep"
#define AUTHORS "Jim meyering & Paul Eggert"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                    program_name);
    else
    {
        printf(_("\
Usage: %s NUMBER[SUFFIX]...\n\
   or: %s OPTION\n\
Pause for NUMBER seconds. SUFFIX may be `s' for seconds (the default),\n\
`m' for minutes, `h' for hours or `d' for days. Unlike most implementations\n\
that requires NUMBER be an integer, here NUMBER may be an arbitrary floating\n\
point number. Given two or more arguments, pause for the amount of time\n\
specified by the sum of their values.\n\
\n\
"),             program_name, program_name);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

/* Given a floating point value *X, and a suffix character, SUFFIX_CHAR,
   scale *X by multiplier implied by SUFFIX_CHAR. SUFFIX_CHAR may
   be the NUL byte or `s' to denote seconds, `m' for minutes, `h' for 
   hours, or `d' for days. If SUFFIX_CHAR is invalid, don't modify *X
   and return false. Otherwise return true. */
static bool apply_suffix(double* x, char suffix_char)
{
    int multiplier;

    switch(suffix_char)
    {
        case 0:
        case 's':
            multiplier = 1;
            break;
        case 'm':
            multiplier = 60;
            break;
        case 'h':
            multiplier = 60 * 60;
            break;
        case 'd':
            multiplier = 60 * 60 * 24;
            break;
        default:
            return false;
    }

    *x *= multiplier;

    return true;
}

int main(int argc, char** argv)
{
    int i;
    double seconds = 0.0;
    bool ok = true;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCLAEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char*)NULL);
    if(getopt_long(argc, argv, "", NULL, NULL) != -1)
        usage(EXIT_FAILURE);

    if(argc == 1)
    {
        error(0, 0, _("missing operand"));
        usage(EXIT_FAILURE);
    }

    for(i = optind; i < argc; i++)
    {
        double s;
        const char* p;
        if(! xstrtod(argv[i], &p, &s, c_strtod)
                /* Nonnegative interval */
                || !(s >= 0)
                /* No extra chars after the number and on optional s,m,h,d char */
                || (*p && *(p + 1))
                /* check any suffix char and update S based on the suffix */
                || ! apply_suffix(&s, *p))
        {
            error(0, 0, _("invalid time interval %s"), quote(argv[i]));
            ok = false;
        }

        seconds += s;
    }

    if(!ok)
        usage(EXIT_FAILURE);

    if(xnanosleep(seconds))
        error(EXIT_FAILURE, errno, _("cannot read realtime clock"));

    exit(EXIT_SUCCESS);
}
