/* chroot -- run command or shell with specical root directory */

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <grp.h>
#include <stdlib.h>
#include <error.h>
#include <unistd.h>

#include "system.h"
#include "long-options.h"
#include "quote.h"
#include "closeout.h"
#include "xstrtol.h"
#include "userspec.h"

#define PROGRAM_NAME "chroot"
#define AUTHORS "Roland McGrath"

#ifndef MAXGID
# define MAXGID GID_T_MAX
#endif

enum
{
    GROUPS = UCHAR_MAX + 1,
    USERSPEC
};

static struct option long_opts[] =
{
    {"groups", required_argument, NULL, GROUPS},
    {"userspec", required_argument, NULL, USERSPEC},
    {GETOPT_HELP_OPTION_DECL},
    {GETOPT_VERSION_OPTION_DECL},
    {NULL, 0, NULL, 0}
};

/* Call setgroups to set the supplementary groups to those listed in GROUPS.
   GROUPS is a comma separated list of supplementary groups (names or numbers).
   Parse that list, converting any names to numbers, and call setgroups on the
   resulting numbers. Upon any failure give a diagnostic and return nonzero.
   Otherwise return zero. */
static int
set_additional_groups(char* groups)
{
    GETGROUPS_T* gids = NULL;
    size_t n_gids_allocated = 0;
    size_t n_gids = 0;
    char* buffer = xstrdup(groups);
    char* tmp;
    int ret = 0;

    for(tmp = strtok(buffer, ","); tmp; tmp = strtok(NULL, ","))
    {
        struct group* g;
        unsigned long int value;

        if(xstrtoul(tmp, NULL, 10, &value, "") == LONGINT_OK
                && value <= MAXGID)
            g = getgrgid(value);
        else
        {
            g = getgrnam(tmp);
            if(g != NULL)
                value = g->gr_gid;
        }
        if(g == NULL)
        {
            error(0, errno, _("invalid group %s"), quote(tmp));
            ret = -1;
            continue;
        }

        if(n_gids == n_gids_allocated)
            gids = X2NREALLOC(gids, &n_gids_allocated);
        gids[n_gids++] = value;
    }

    if(ret == 0 && n_gids == 0)
    {
        error(0, 0, _("invalid group list %s"), quote(groups));
        return -1;
    }

    if(ret == 0)
    {
        ret = setgroups(n_gids, gids);
        if(ret)
            error(0, errno, _("failed to set additional groups"));
    }

    free(buffer);
    free(gids);
    return ret;
}

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                    program_name);
    else
    {
        printf(_("\
Usage: %s [OPTION] NEWROOT [COMMAND [ARG]...]\n\
   or: %s OPTION\n\
"), program_name, program_name);
        fputs(_("\
Run COMMAND with root directory set to NEWROOT.\n\
\n\
"), stdout);

        fputs(_("\
  --userspec=USER:GROUP specify user and group (ID or name) to user\n\
  --groups=G_LIST       specify supplementary groups as g1,g2,...,gN\n\
"), stdout);

        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        fputs(_("\
\n\
If no command is given, run ``${SHELL} -i'' (default: /bin/sh).\n\
"), stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    int c;
    char* userspec = NULL;
    char* groups = NULL;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    initialize_exit_failure(EXIT_CANCELED);
    atexit(close_stdout);

    parse_long_options(argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
                        usage, AUTHORS, (char*)NULL);

    while((c = getopt_long(argc, argv, "+", long_opts, NULL)) != -1)
    {
        switch(c)
        {
            case USERSPEC:
                userspec = optarg;
                break;
            case GROUPS:
                groups = optarg;
                break;
            default:
                usage(EXIT_CANCELED);
        }
    }

    if(argc <= optind)
    {
        error(0, 0, _("missing operand"));
        usage(EXIT_CANCELED);
    }

    if(chroot(argv[optind]) != 0)
        error(EXIT_CANCELED, errno, _("cannot change root directory to %s"),
                argv[optind]);

    if(chdir("/"))
        error(EXIT_CANCELED, errno, _("cannot chdir to root directory"));

    if(argc == optind + 1)
    {
        /* No command. Run an interactive shell */
        char* shell = getenv("SHELL");
        if(shell == NULL)
            shell = bad_cast("/bin/sh");
        argv[0] = shell;
        argv[1] = bad_cast("-i");
        argv[2] = NULL;
    }
    else
    {
        /* The following arguments give the command */
        argv += optind + 1;
    }

    bool fail = false;

    /* Attempt to set all three: supplementary groups, group ID, user ID.
       Diagnose any failures. If any have failed, exit before execvp */
    if(userspec)
    {
        uid_t uid = -1;
        gid_t gid = -1;
        char* user;
        char* group;
        char* err = parse_user_spec(userspec, &uid, &gid, &user, &group);

        if(err)
            error(EXIT_CANCELED, errno, "%s", err);

        free(user);
        free(group);

        if(groups && set_additional_groups(groups))
            fail = true;

        if(gid != (gid_t) -1 && setgid(gid))
        {
            error(0, errno, _("failed to set group-ID"));
            fail = true;
        }

        if(uid != (uid_t) -1 && setuid(uid))
        {
            error(0, errno, _("failed to set user-ID"));
            fail = true;
        }
    }
    else
    {
        /* Yes, this call is identical to the one above.
           However, when --userspec and --groups groups are used together,
           we don't want to call this function until after parsing USER:GROUP,
           and it must be called before setuid */
        if(groups && set_additional_groups(groups))
            fail = true;
    }

    if(fail)
        exit(EXIT_CANCELED);

    /* Execute the given command */
    execvp(argv[0], argv);

    {
        int exit_status = (errno == ENOENT ? EXIT_ENOENT : EXIT_CANNOT_INVOKE);
        error(0, errno, _("failed to run command %s"), quote(argv[0]));
        exit(exit_status);
    }
}

