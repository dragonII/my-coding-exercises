/* mkfifo -- make fifo's (named pipes) */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <selinux/selinux.h>
#include <sys/stat.h>
#include <linux/stat.h>

#include "system.h"
#include "quote.h"
#include "version.h"
#include "closeout.h"
#include "modechange.h"
#include "sys_stat.in.h"

#define PROGRAM_NAME "mkfifo"
#define AUTHORS "David MacKenzie"

static struct option longopts[] =
{
    {GETOPT_SELINUX_CONTEXT_OPTION_DECL},
    {"mode", required_argument, NULL, 'm'},
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
        printf(_("Usage: %s [OPTION] ... NAME ...\n"), program_name);
        fputs(_("\
Create named pipes (FIFOs) with the given NAMEs.\n\
\n\
"), stdout);
        fputs(_("\
Mandatory arguments to long options are mandatory for short options too.\n\
"), stdout);
        fputs(_("\
    -m, --mode=MODE     set file permission bits to MODE, not a=rw - umask\n\
"), stdout);
        fputs(_("\
    -Z, --context=CTX   set the SELinux security context of each NAME to CTX\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
    mode_t newmode;
    char* specified_mode = NULL;
    int exit_status = EXIT_SUCCESS;
    int optc;
    security_context_t scontext = NULL;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    while((optc = getopt_long(argc, argv, "m:Z:", longopts, NULL)) != -1)
    {
        switch(optc)
        {
            case 'm':
                specified_mode = optarg;
                break;
            case 'Z':
                scontext = optarg;
                break;
            case_GETOPT_HELP_CHAR;

            case_GETOPT_VERSION_CHAR(PROGRAM_NAME, AUTHORS);
            default:
                usage(EXIT_FAILURE);
        }
    }

    printf("optind = %d\n", optind);
    if(optind == argc)
    {
        error(0, 0, _("missing operand"));
        usage(EXIT_FAILURE);
    }

    if(scontext && setfscreatecon(scontext) < 0)
        error(EXIT_FAILURE, errno,
                _("failed to set default file creation context to %s"),
                quote(scontext));

    newmode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if(specified_mode)
    {
        struct mode_change* change = mode_compile(specified_mode);
        if(!change)
            error(EXIT_FAILURE, 0, _("invalid mode"));
        newmode = mode_adjust(newmode, false, umask(0), change, NULL);
        free(change);
        if(newmode & ~S_IRWXUGO)
            error(EXIT_FAILURE, 0, 
                    _("mode must specify only file permission bits"));
    }

    for(; optind < argc; ++optind)
    {
        printf("argv[%d]: %s\n", optind, argv[optind]);
        if(mkfifo(argv[optind], newmode) != 0)
        {
            error(0, errno, _("cannot create fifo %s"), quote(argv[optind]));
            exit_status = EXIT_FAILURE;
        }
    }

    exit(exit_status);
}
