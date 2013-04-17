/* comm -- compare two sorted files line by line */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <stdbool.h>
#include <string.h>

#include "system.h"
#include "quote.h"
#include "closeout.h"

#define PROGREAM_NAME "comm"
#define AUTHORS "Richard Stallman & David MacKenzie"

/* Undefine, to avoid warning about redefinition on some systems */
#undef min
#define min(x, y) ((x) < (y) ? (x) : (y))

#ifndef STREQ
#define STREQ(a, b) (strcmp(a, b) == 0)
#endif

/* True if the LC_COLLATE locale is hard */
static bool hard_LC_COLLATE;

/* If true, print lines that are found only in file 1 */
static bool only_file_1;

/* If true, print lines that are found only in file 2 */
static bool only_file_2;

/* If true, print lines that are found in both files */
static bool both;

/* If nonzero, we have seen at least one unpairable line */
static bool seen_unpairable;

/* If nonzero, we have warned about disorder in that file */
static bool issued_disorder_warning[2];

/* If nonzero, check that the input is correctly ordered */
static enum
{
    CHECK_ORDER_DEFAULT,
    CHECK_ORDER_ENABLED,
    CHECK_ORDER_DISABLED,
} check_input_order;

/* Output columns will be delimited with this string, which may be set
   on the command-line with --output-delimiter=STR. The default is a
   single TAB character */
static char const *delimiter;

/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1 */
enum
{
    CHECK_ORDER_OPTION = CHAR_MAX + 1,
    NOCHECK_ORDER_OPTION,
    OUTPUT_DELIMITER_OPTION
};

static struct option const long_opts[] =
{
    {"check-order", no_argument, NULL, CHECK_ORDER_OPTION},
    {"nocheck-order", no_argument, NULL, NOCHECK_ORDER_OPTION},
    {"output-delimiter", required_argument, NULL, OUTPUT_DELIMITER_OPTION},
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
Usage: %s [OPTION]... FILE1 FILE2\n\
"),
            program_name);
        fputs(_("\
Compare sorted files FILE1 and FILE2 line by line.\n\
"), stdout);
        fputs(_("\
\n\
With no options, produce three-colume output. Column one contains\n\
lines unique to FILE1, column two contains lines unique to FILE2,\n\
and column three contains line common to both files.\n\
"), stdout);
        fputs(_("\
\n\
  -1            suppress column 1 (lines unique to FILE1)\n\
  -2            suppress column 2 (lines unique to FILE2)\n\
  -3            suppress column 3 (lines that appear in both files)\n\
"), stdout);
        fputs(_("\
\n\
  --check-order     check that the input is correctly sorted, even\n\
                        if all input lines are pariable\n\
  --nocheck-order   do not check that the input is correctly sorted\n\
"), stdout);
        fputs(_("\
  --output-delimiter=STR    separate columns with STR\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        fputs(_("\
\n\
Note, comparisons honor the rules specified by `LC_COLLATE'.\n\
"), stdout);
        printf(_("\
\n\
Examples:\n\
  %s -12 file1 file2    Print only lines present in both file1 and file2.\n\
  %s -3  file1 file2    Print lines only in file1 not in file2, and vice versa.\n\
"),
                program_name, program_name);
        emit_ancillary_info();
    }
    exit(status);
}


int main(int argc, char **argv)
{
    int c;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    hard_LC_COLLATE = hard_locale(LC_COLLATE);

    atexit(close_stdout);

    only_file_1 = true;
    only_file_2 = true;
    both = true;

    seen_unpairable = false;
    issued_disorder_warning[0] = issued_disorder_warning[1] = false;
    check_input_order = CHECK_ORDER_DEFAULT;

    while((c = getopt_long(argc, argv, "123", long_opts, NULL)) != -1)
    {
        switch(c)
        {
            case '1':
                only_file_1 = false;
                break;
            case '2':
                only_file_2 = false;
                break;

            case '3':
                both = false;
                break;

            case NOCHECK_ORDER_OPTION:
                check_input_order = CHECK_ORDER_DISABLED;
                break;

            case CHECK_ORDER_OPTION:
                check_input_order = CHECK_ORDER_ENABLED;
                break;

            case OUTPUT_DELIMITER_OPTION:
                if(delimiter && !STREQ (delimiter, optarg))
                    error(EXIT_FAILURE, 0, _("multiple delimiters specified"));
                delimiter = optarg;
                if(!*delimiter)
                {
                    error(EXIT_FAILURE, 0, _("empty %s not allowed"),
                        quote("--output-delimiter"));
                }
                break;
            case_GETOPT_HELP_CHAR;

            case_GETOPT_VERSION_CHAR(PROGREAM_NAME, AUTHORS);

            default:
                usage(EXIT_FAILURE);
        } // switch
    } // while

    if(argc - optind < 2)
    {
        if(argc <= optind)
            error(0, 0, _("missing operand"));
        else
            error(0, 0, _("missing operand after %s"), quote(argv[argc - 1]));
        usage(EXIT_FAILURE);
    }

    if(argc - optind > 2)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind + 2]));
        usage(EXIT_FAILURE);
    }

    /* The default delimiter is a TAB */
    if(!delimiter)
        delimiter = "\t";

    compare_files(argv + optind);

    if(issued_disorder_warning[0] || issued_disorder_warning[1])
        exit(EXIT_FAILURE);
    else
        exit(EXIT_SUCCESS);
}
