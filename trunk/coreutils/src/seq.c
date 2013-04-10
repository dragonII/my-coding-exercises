/* seq - print sequence of numbers to standard output */

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include "system.h"
#include "quote.h"
#include "xstrtod.h"
#include "closeout.h"

/* Roll our own isfinite rather than using <math.h>, so that we don't
   have to worry about linking -lm just for isfinite */
#ifndef isfinite
# define isfinite(x) ((x) * 0 == 0)
#endif

#define PROGRAM_NAME "seq"
#define AUTHORS "Ulrich Drepper"

/* if true, print all number with equal width */
static bool equal_width;

/* the string used to separate two numbers */
static char *separator;

/* the string output after all numbers have been output.
   Usually "\n" or "\0" */
static char terminator[] = "\n";

static struct option long_options[] =
{
    {"equal-width", no_argument, NULL, 'w'},
    {"format", required_argument, NULL, 'f'},
    {"separator", required_argument, NULL, 's'},
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
Usage: %s [OPTION]... LAST\n\
   or: %s [OPTION]... FIRST LAST\n\
   or: %s [OPTION]... FIRST INCREMENT LAST\n\
"), program_name, program_name, program_name);
        fputs(_("\
Print numbers from FIRST to LAST, in steps of INCREMENT.\n\
\n\
  -f, --format=FORMAT       use printf style floating-point FORMAT\n\
  -s, --separator=STRING    use STRING to separate numbers (default: \\n)\n\
  -w, --equal-width         equalize width by padding with leading zeroes\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        fputs(_("\
\n\
If FIRST or INCREMENT is omitted, it defaults to 1. That is, an\n\
omitted INCREMENT defaults to 1 even when LAST is smaller than FIRST.\n\
FIRST, INCREMENT, and LAST are interpreted as floating point values.\n\
INCREMENT is usually positive if FIRST is smaller than LAST, and\n\
INCREMENT is usually negative if FIRST is greater than LAST.\n\
"), stdout);
        fputs(_("\
FORMAT must be suitable for printing one argument of type `double';\n\
it defaults to %.PRECf if FIRST, INCREMENT, and LAST are all fixed point\n\
decimal numbers with maximum precision PREC, and to %g otherwise.\n\
"), stdout);
        emit_ancillary_info();
    }
    exit(status);
}

/* A command-line operand */
struct operand
{
    /* Its value, converted to 'long double' */
    long double value;

    /* Its first width, if it were printed out in a form similar to its
       input form. An input like "-.1" is treated like "-0.1", and an
       input like "1." is treated like "1", but otherwise widths are
       left alone. */
    size_t width;

    /* numbers of digits after the decimal point, or INT_MAX if the
       number can't easily be expressed as a fixed-point number. */
    int precision;
};

typedef struct operand operand;

/* Description of what a number-generating format will generate */
struct layout
{
    /* number of bytes before and after the number */
    size_t prefix_len;
    size_t suffix_len;
};

int main(int argc, char** argv)
{
    int optc;
    operand first = { 1, 1, 0 };
    operand step = { 1, 1, 0 };
    operand last;
    struct layout layout = { 0, 0 };

    /* the printf(3) format used for output */
    char* format_str = NULL;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    equal_width = false;
    separator = "\n";

    /* we have to handle negative numbers in the command line but this
       conflicts with the command line arguments. So explicitly check first
       whether the next argument looks like a negative number */
    while(optind < argc)
    {
        if(argv[optind][0] == '-'
            && ((optc = argv[optind][1]) == '.' || ISDIGIT(optc)))
        {
            /* means negative number */
            break;
        }

        optc = getopt_long(argc, argv, "+f:s:w", long_options, NULL);
        if(optc == -1)
            break;

        switch(optc)
        {
            case 'f':
                format_str = optarg;
                break;
            case 's':
                separator = optarg;
                break;
            case 'w':
                equal_width = true;
                break;

            case_GETOPT_HELP_CHAR;

            case_GETOPT_VERSION_CHAR(PROGRAM_NAME, AUTHORS);

            default:
                usage(EXIT_FAILURE);
        }
    }
    if(argc - optind > 3)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind + 3]));
        usage(EXIT_FAILURE);
    }

    if(format_str)
        format_str = long_double_format(format_str, &layout);

    last = scan_arg(argv[optind++]);

    if(optind < argc)
    {
        first = last;
        last = scan_arg(argv[optind++]);

        if(optind < argc)
        {
            step = last;
            last = scan_arg(argv[optind++]);
        }
    }

    if(format_str != NULL && equal_width)
    {
        error(0, 0, _("\
format string may not be specified when printing equal width strings"));
        usage(EXIT_FAILURE);
    }

    if(format_str == NULL)
        format_str = get_default_format(first, step, last);

    print_numbers(format_str, layout, first.value, step.value, last.value);

    exit(EXIT_SUCCESS);
}
