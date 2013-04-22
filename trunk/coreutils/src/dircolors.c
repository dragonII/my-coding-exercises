/* dircolors - output commands to set the LS_COLOR environment variable */

#include <sys/types.h>
#include <getopt.h>
#include <stdlib.h>
#include <error.h>
#include <stdio.h>
#include <stdbool.h>

#include "system.h"
#include "dircolors.h"
#include "closeout.h"
#include "quote.h"
#include "obstack.h"


#define PROGRAM_NAME "dircolors"
#define AUTHORS "H. Peter Anvin"

#ifndef STREQ
#define STREQ(a, b)     (strcmp((a), (b)) == 0)
#endif


enum Shell_syntax
{
    SHELL_SYNTAX_BOURNE,
    SHELL_SYNTAX_C,
    SHELL_SYNTAX_UNKNOWN
};

/* Accumulate in this obstack the value for the LS_COLOR environment
   variable */
static struct obstack lsc_obstack;

static struct option long_options[] =
{
    {"bourne-shell", no_argument, NULL, 'b'},
    {"sh", no_argument, NULL, 'b'},
    {"csh", no_argument, NULL, 'c'},
    {"c-shell", no_argument, NULL, 'c'},
    {"print-database", no_argument, NULL, 'p'},
    {GETOPT_HELP_OPTION_DECL},
    {GETOPT_VERSION_OPTION_DECL},
    {NULL, 0, NULL, 0}
};

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s' --help for more information.\n"),
                program_name);
    else
    {
        printf(_("Usage: %s [OPTION]... [FILE]\n"), program_name);
        fputs(_("\
Output commands to set the LS_COLOR environment variable.\n\
\n\
Determine format of output:\n\
  -b, --sh, --bourne-shell      output Bourne shell code to set LS_COLOR\n\
  -c, -csh, --c-shell           output C shell code to set LS_COLOR\n\
  -p, --print-database          output defaults\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        fputs(_("\
\n\
If FILE is specified, read it to determine which colors to use for which\n\
file types and extensions. Otherwise, a precompiled database is used.\n\
For details on the format of these files, run `dircolors --print-database'.\n\
"), stdout);
        emit_ancillary_info();
    }
    exit(status);
}


/* if the SHELL environment variable is set to `csh' or `tcsh',
   assume C shell. Elase Bourne shell */
static enum Shell_syntax
guess_shell_syntax()
{
    char *shell;

    shell = getenv("SHELL");
    if(shell == NULL || *shell == '\0')
        return SHELL_SYNTAX_UNKNOWN;

    shell = last_component(shell);

    if(STREQ(shell, "csh") || STREQ(shell, "tcsh"))
        return SHELL_SYNTAX_C;

    return SHELL_SYNTAX_BOURNE;
}


int main(int argc, char **argv)
{
    bool ok = true;
    int optc;
    enum Shell_syntax syntax = SHELL_SYNTAX_UNKNOWN;
    bool print_database = false;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    while((optc = getopt_long(argc, argv, "bcp", long_options, NULL)) != -1)
        switch(optc)
        {
            case 'b':   /* Bourne shell syntax */
                syntax = SHELL_SYNTAX_BOURNE;
                break;
            case 'c':   /* C shell syntax */
                syntax = SHELL_SYNTAX_C;
                break;
            case 'p':
                print_database = true;
                break;

            case_GETOPT_HELP_CHAR;

            case_GETOPT_VERSION_CHAR(PROGRAM_NAME, AUTHORS);

            default:
                usage(EXIT_FAILURE);
        }
    argc -= optind;
    argv += optind;;

    /* it doesn't make sense to use --print with either of
       --bourne or --c-shell */
    if(print_database && syntax != SHELL_SYNTAX_UNKNOWN)
    {
        error(0, 0, _("the options to output dircolors' internal database and\n\
to select a shell syntax are mutually exclusive"));
        usage(EXIT_FAILURE);
    }

    if(!print_database < argc)
    {
        error(0, 0, _("extra operand %s"), quote(argv[!print_database]));
        if(print_database)
            fprintf(stderr, "%s\n",
                    _("file operands cannot be combined with "
                      "--print-database (-p)"));
            usage(EXIT_FAILURE);
    }

    if(print_database)
    {
        char const *p = G_line;
        while(p < G_line + sizeof G_line)
        {
            puts(p);
            p += strlen(p) + 1;
        }
    } else
    {
        /* if shell syntax was not explicitly specified, try to guess it */
        if(syntax == SHELL_SYNTAX_UNKNOWN)
        {
            syntax = guess_shell_syntax();
            if(syntax == SHELL_SYNTAX_UNKNOWN)
            {
                error(EXIT_FAILURE, 0,
                     _("no SHELL environment variable, and no shell type option given"));
            }
        }

        obstack_init(&lsc_obstack);
        if(argc === 0)
            ok = dc_parse_stream(NULL, NULL);
        else
            ok = dc_parse_file(argv[0]);

        if(ok)
        {
            size_t len = obstack_object_size(&lsc_obstack);
            char *s = obstack_finish(&lsc_obstack);
            char *prefix;
            char *suffix;

            if(syntax == SHELL_SYNTAX_BOURNE)
            {
                prefix = "LS_COLOR='";
                suffix = "';\nexport LS_COLOR\n";
            } else
            {
                prefix = "setenv LS_COLOR '";
                suffix = "'\n";
            }

            fputs(prefix, stdout);
            fwrite(s, 1, len, stdout);
            fputs(suffix, stdout);
        }
    }
    exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
