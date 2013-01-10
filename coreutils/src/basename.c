/* basename -- strip directory and suffix from file names

   Usage: basename name [suffix]
   NAME is a file; SUFFIX is a suffix to strip from it.

   basename /usr/foo/lossage/functions.l
   => functions.l
   basename /usr/foo/lossage/functions.l .l
   => functions
   basename functions.lisp p
   => functions.lis */

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <stdlib.h>

#include "system.h"
#include "version.h"
#include "closeout.h"
#include "long-options.h"
#include "quote.h"
#include "dirname.h"
#include "lib-basename.h"
#include "stripslash.h"

#define PROGRAM_NAME "basename"
#define AUTHORS "Daved MacKenzie"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("\
Usage: %s NAME [SUFFIX]\n\
   or: %s OPTION\n\
"),
                program_name, program_name);
        fputs(_("\
Print NAME with any leading directory components removed.\n\
If specified, also remove trailing SUFFIX.\n\
\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        printf(_("\
\n\
Examples:\n\
  %s /usr/bin/sort      Output \"sort\".\n\
  %s include/stdio.h    Output \"stdio\".\n\
"),
                program_name, program_name);
        emit_ancillary_info();
    }
    exit(status);
}

/* Remove SUFFIX from the end of NAME if it is threr, unless NAME
   consists entirely of SUFFIX. */
static void remove_suffix(char* name, const char* suffix)
{
    char* np;
    const char* sp;

    np = name + strlen(name);
    sp = suffix + strlen(suffix);

    while(np > name && sp > suffix)
        if(*--np != *--sp)
            return;
    if(np > name)
        *np = '\0';
}

int main(int argc, char** argv)
{
    char* name;

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

    if(optind + 2 < argc)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind + 2]));
        usage(EXIT_FAILURE);
    }

    name = base_name(argv[optind]);
    strip_trailing_slashes(name);

    /* Per POSIX, `basename // /' must return `//' on platforms with
       distinct //. On platforms with drive letters, this generalizes
       to making `basename c: :' return `c:'. This rule is captured by
       skipping suffix stripping if base_name returned an absolute path
       or a drive letter (only possible if name is a file-system
       root). */
    if(argc == optind + 2 && IS_RELATIVE_FILE_NAME(name)
        && !FILE_SYSTEM_PREFIX_LEN(name))
        remove_suffix(name, argv[optind + 1]);

    puts(name);
    free(name);

    exit(EXIT_SUCCESS);
}
