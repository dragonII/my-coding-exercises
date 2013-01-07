/* whoami -- print effective userid
   Equivalent to `id -un'. */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <getopt.h>
#include <error.h>
#include <stdlib.h>

#include "long-options.h"
#include "quote.h"
#include "system.h"
#include "closeout.h"
#include "version.h"

#define PROGRAM_NAME "whoami"
#define AUTHORS "Richard Mlynarik"

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("Usage: %s [OPTION]...\n"), program_name);
        fputs(_("\
Print the user name associated with the current effective user ID.\n\
Same as id -un.\n\
\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    struct passwd *pw;
    uid_t uid;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char*)NULL);
    if(getopt_long(argc, argv, "", NULL, NULL) != -1)
        usage(EXIT_FAILURE);

    if(optind != argc)
    {
        error(0, 0, _("extra operand %s"), quote(argv[optind]));
        usage(EXIT_FAILURE);
    }

    uid = geteuid();
    pw = getpwuid(uid);
    if(pw)
    {
        puts(pw->pw_name);
        exit(EXIT_SUCCESS);
    }
    fprintf(stderr, _("%s: cannot find name for user ID %lu\n"),
            program_name, (unsigned long int)uid);
    exit(EXIT_FAILURE);
}

