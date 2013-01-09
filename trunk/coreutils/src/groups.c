/* groups -- print the groups a user is in */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <error.h>
#include <getopt.h>
#include <stdlib.h>
#include <pwd.h>

#include "system.h"
#include "version.h"
#include "closeout.h"
#include "group-list.h"

#define PROGRAM_NAME "groups"
#define AUTHORS "David MacKenzie & James Youngman"

static struct option longopts[] =
{
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
        printf(_("Usage: %s [OPTION]... [USERNAME]...\n"), program_name);
        fputs(_("\
Print group memberships for each USERNAME or, if no USERNAME is specified, for\n\
the current process (which may differ if the groups database has changed).\n"),
                stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    int optc;
    bool ok = true;
    gid_t rgid, egid; // real and effective
    uid_t ruid;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    /* Processing the arguments this way makes groups.c behave differently to
       groups.sh if one of the arguments is "--". */
    while((optc = getopt_long(argc, argv, "", longopts, NULL)) != -1)
    {
        switch(optc)
        {
            case_GETOPT_HELP_CHAR;
            case_GETOPT_VERSION_CHAR(PROGRAM_NAME, AUTHORS);
            default:
                usage(EXIT_FAILURE);
        }
    }

    if(optind == argc)
    {
        /* no arguments. Divulge the details of the current process */
        ruid = getuid();
        egid = getegid();
        rgid = getgid();

        if(!print_group_list(NULL, ruid, rgid, egid, true))
            ok = false;
        putchar('\n');
    }
    else
    {
        /* At least one arguments. Divulge the details of the specified users. */
        while(optind < argc)
        {
            struct passwd *pwd = getpwnam(argv[optind]);
            if(pwd == NULL)
                error(EXIT_FAILURE, 0, _("%s: No such user"), argv[optind]);
            ruid = pwd->pw_uid;
            rgid = egid = pwd->pw_gid;

            printf("%s : ", argv[optind]);
            if(!print_group_list(argv[optind++], ruid, rgid, egid, true))
                ok = false;
            putchar('\n');
        }
    }
    exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
