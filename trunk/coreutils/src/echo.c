/* echo.c, derived from echo echo.c in Bash. */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "closeout.h"

#define PROGRAM_NAME "echo"
#define AUTHORS "Brian Fox & Chet Ramey"

/* If true, interpret backslash escapes by default */
#ifndef DEFAULT_ECHO_TO_XPG
enum { DEFAULT_ECHO_TO_XPG = false };
#endif

#ifndef STREQ
#define STREQ(x, y) (strcmp((x), (y)) == 0)
#endif

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                    program_name);
    else
    {
        printf(_("\
Usage: %s [SHORT-OPTION]... [STRING]...\n\
   or: %s LONG-OPTION\n\
"), program_name, program_name);
        fputs(_("\
Echo the STRING(s) to standard output.\n\
\n\
  -n            do not output the trailing newline\n\
"), stdout);
        fputs(_(DEFAULT_ECHO_TO_XPG
                ? N_("\
  -e            enable interpretation of backslash escapes (default)\n\
  -E            disable interpretation of backslash escapes\n")
                : N_("\
  -e            enable interpretation of backslash escapes\n\
  -E            disable interpretation of backslash escapes (default)\n")),
                stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        fputs(_("\
\n\
If -e is in effect, the following sequences are recognized:\n\
\n\
"), stdout);
        fputs(_("\
  \\\\      backslash\n\
  \\a       alert (BEL)\n\
  \\b       backspace\n\
  \\c       produce no further output\n\
  \\e       escape\n\
  \\f       form feed\n\
  \\n       new line\n\
  \\r       carriage return\n\
  \\t       horizontal tab\n\
  \\v       vertical tab\n\
"), stdout);
        fputs(_("\
  \\0NNN    byte with octal value NNN (1 to 3 digits)\n\
  \\xHH     byte with hexadecimal value HH (1 to 2 digits)\n\
"), stdout);
        printf(USAGE_BUILTIN_WARNING, PROGRAM_NAME);
        emit_ancillary_info();
    }
    exit(status);
}

/* Convert C from hexadecimal character to integer */
static int
hextobin(unsigned char c)
{
    switch(c)
    {
        default: return c - '0';
        case 'a': case 'A': return 10;
        case 'b': case 'B': return 11;
        case 'c': case 'C': return 12;
        case 'd': case 'D': return 13;
        case 'e': case 'E': return 14;
        case 'f': case 'F': return 15;
    }
}

/* Print the words in LIST to standard output. If the first word is
   `-n', then don't print a trailing newline. We also support the
   echo syntax from Version 9 unix systems. */
int main(int argc, char** argv)
{
    bool display_return = true;
    bool allow_options =
        (! getenv("POSIXLY_CORRECT")
            || (! DEFAULT_ECHO_TO_XPG && 1 < argc && STREQ(argv[1], "-n")));

    /* System V machines already have a /bin/sh with a v9 behavior.
       Use the identical behavior for these machines so that the
       existing system shell scripts won't barf */
    bool do_v9 = DEFAULT_ECHO_TO_XPG;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    /* We directly parse options, rather than use parse_long_options, in
       order to avoid accepting abbreviations. */
    if(allow_options && argc == 2)
    {
        if(STREQ(argv[1], "--help"))
            usage(EXIT_SUCCESS);

        if(STREQ(argv[1], "--version"))
        {
            version_etc(stdout, PROGRAM_NAME, PACKAGE_NAME, Version, AUTHORS,
                        (char*)NULL);
            exit(EXIT_SUCCESS);
        }
    }

    --argc;
    ++argv;

    if(allow_options)
        while(argc > 0 && *argv[0] == '-')
        {
            char* temp = argv[0] + 1;
            size_t i;

            /* If it appears that we are handling options, then make sure that
               all of the options specified are actually valid. Otherwise, the
               string should just be echoed. */

            for(i = 0; temp[i]; i++)
                switch(temp[i])
                {
                    case 'e': case 'E': case 'n':
                        break;
                    default:
                        goto just_echo;
                }

            if(i == 0)
                goto just_echo;

            /* All of the options in TEMP are valid options to ECHO.
               Handle them */
            while(*temp)
                switch(*temp++)
                {
                    case 'e':
                        do_v9 = true;
                        break;

                    case 'E':
                        do_v9 = false;
                        break;

                    case 'n':
                        display_return = false;
                        break;
                }

            argc--;
            argv++;
        }

just_echo:
        if(do_v9)
        {
            while(argc > 0)
            {
                char* s = argv[0];
                unsigned char c;

                while((c = *s++))
                {
                    if(c == '\\' && *s)
                    {
                        switch(c = *s++)
                        {
                            case 'a': c = '\a'; break;
                            case 'b': c = '\b'; break;
                            case 'c': exit(EXIT_SUCCESS);
                            case 'e': c = '\x1B'; break;
                            case 'f': c = '\f'; break;
                            case 'n': c = '\n'; break;
                            case 'r': c = '\r'; break;
                            case 't': c = '\t'; break;
                            case 'v': c = '\v'; break;
                            case 'x':
                            {
                                unsigned char ch = *s;
                                if(! isxdigit(ch))
                                    goto not_an_escape;
                                s++;
                                c = hextobin(ch);
                                ch = *s;
                                if(isxdigit(ch))
                                {
                                    s++;
                                    c = c * 16 + hextobin(ch);
                                }
                            }
                                break;
                            case '0':
                                c = 0;
                                if(!('0' <= *s && *s <= '7'))
                                    break;
                                c = *s++;
                                /* Fall through */
                            case '1': case '2': case '3':
                            case '4': case '5': case '6': case '7':
                                c -= '0';
                                if('0' <= *s && *s <= '7')
                                    c = c * 8 + (*s++ - '0');
                                if('0' <= *s && *s <= '7')
                                    c = c * 8 + (*s++ - '0');
                                break;

                            case '\\': break;

                            not_an_escape:
                            default: putchar('\\'); break;
                        }
                    }
                    putchar(c);
                }
                argc--;
                argv++;
                if(argc > 0)
                    putchar(' ');
            }
        }
        else
        {
            while(argc > 0)
            {
                fputs(argv[0], stdout);
                argc--;
                argv++;
                if(argc > 0)
                    putchar(' ');
            }
        }

        if(display_return)
            putchar('\n');
        exit(EXIT_SUCCESS);
}
