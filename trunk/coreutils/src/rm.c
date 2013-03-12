/*  `rm' file deletion utility for GNU. */

#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <assert.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>

#include "system.h"
#include "closeout.h"
#include "quote.h"
#include "argmatch.h"
#include "remove.h"
#include "priv-set.h"
#include "root-dev-ino.h"
#include "yesno.h"

#define PROGROM_NAME "rm"
#define AUTHORS "Paul Rubin, David MacKenzie, Richard M. Stallman & Jim Meyering"

enum
{
    INTERACTIVE_OPTION = CHAR_MAX + 1,
    ONE_FILE_SYSTEM,
    NO_PRESERVE_ROOT,
    PRESERVE_ROOT,
    PRESUME_INPUT_TTY_OPTION
};

enum interactive_type
{
    interactive_never,      /* 0: no option or --interactive=never */
    interactive_once,       /* 1: -I or --interactive=once */
    interactive_always      /* 2: default, -i or --interactive=always */
};

static struct option long_opts[] =
{
    {"directory", no_argument, NULL, 'd'},
    {"force", no_argument, NULL, 'f'},
    {"interactive", optional_argument, NULL, INTERACTIVE_OPTION},

    {"one-file-system", no_argument, NULL, ONE_FILE_SYSTEM},
    {"no-preserve-root", no_argument, NULL, NO_PRESERVE_ROOT},
    {"preserve-root", no_argument, NULL, PRESERVE_ROOT},

    /* This is solely for testing. Do not document */
    /* It is relatively difficult to ensure that there is a tty on stdin.
       Since rm acts differently depending on that, without this option,
       it'd be harder to test the parts of rm that depend on that setting */
    {"-presume-input-tty", no_argument, NULL, PRESUME_INPUT_TTY_OPTION},

    {"recursive", no_argument, NULL, 'r'},
    {"verbose", no_argument, NULL, 'v'},
    {GETOPT_HELP_OPTION_DECL},
    {GETOPT_VERSION_OPTION_DECL},
    {NULL, 0, NULL, 0}
};

static char* interactive_args[] =
{
    "never", "no", "none",
    "once",
    "always", "yes", NULL
};

static enum interactive_type interactive_types[] =
{
    interactive_never, interactive_never, interactive_never,
    interactive_once,
    interactive_always, interactive_always
};

ARGMATCH_VERIFY (interactive_args, interactive_types);

/* Advise the user about invalid usages like "rm -foo" if the file
   "-foo" exists, assuming ARGC and ARGV are as with `main' */
static void
diagnostic_leading_hyphen(int argc, char** argv)
{
    /* OPTIND is unreliable, so iterate through the arguments looking
       for a file name that looks like an option */
    int i;

    for(i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        struct stat st;

        if(arg[0] == '-' && arg[1] && lstat(arg, &st) == 0)
        {
            fprintf(stderr,
                    _("Try `%s ./%s' to remove the file %s.\n"),
                    argv[0],
                    quote(arg), quote(arg));
            break;
        }
    }
}

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("Usage: %s [OPTION]... FILE...\n"), program_name);
        fputs(_("\
Remove (unlink) the FILE(s).\n\
\n\
  -f, --force           ignore nonexistent files, never prompt\n\
  -i                    prompt before every removal\n\
"), stdout);
        fputs(_("\
  -I                    prompt once before removing more than three files, or\n\
                          when removing recursively. Less instrusive than -i,\n\
                          while still giving protection against most mistakes\n\
     --interactive[=when]  prompt according to WHEN: never, once (-I), or\n\
                           always (-i). Without WHEN, prompt always\n\
"), stdout);
        fputs(_("\
     --one-file-system  when removing a hierarchy recursively, skip any\n\
                        directory that is on a file system different from\n\
                        that of the corresponding command line argument\n\
"), stdout);
        fputs(_("\
     --no-preserve-root do not treat `/' specially\n\
     --preserve-root    do not remove `/' (default)\n\
  -r, -R, --recursive   remove directories and their contents recursively\n\
  -v, --verbose         explain what is being done\n\
"), stdout);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        fputs(_("\
\n\
By default, rm does not remove directories. Use the --recursive (-r or -R)\n\
option to remove each listed directory, too, along with all of its contents.\n\
"), stdout);
        printf(_("\
\n\
To remove a file whose name starts with a `-', for example, `-foo',\n\
use one of these commands:\n\
  %s -- -foo\n\
\n\
  %s ./-foo\n\
"), 
                program_name, program_name);
        fputs(_("\
\n\
Note that if you use rm to remove a file, it is usally possible to recover\n\
the contents of that file. If you want more assurance that the contents are\n\
truly unrecoverable, consider using shred.\n\
"), stdout);
        emit_ancillary_info();
    }
    exit(status);
}

static void
rm_option_init(struct rm_options* x)
{
    x->ignore_missing_files = false;
    x->interactive = RMI_SOMETIMES;
    x->one_file_system = false;
    x->recursive = false;
    x->root_dev_ino = NULL;
    x->stdin_tty = isatty(STDIN_FILENO);
    x->verbose = false;

    /* Since this program exits immediately after calling `rm', rm need not
       expend unnecessary effort to preserve the initial working directory. */
    x->require_restore_cwd = false;
}

int main(int argc, char** argv)
{
    bool preserve_root = true;
    struct rm_options x;
    bool prompt_once = false;
    int c;

    initialize_main(&argc, &argv);
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    atexit(close_stdout);

    rm_option_init(&x);

    /* Try to disable the ability to unlink a directory */
    priv_set_remove_linkdir();

    while((c = getopt_long(argc, argv, "dfirvIR", long_opts, NULL)) != -1)
    {
        switch(c)
        {
            case 'd':
                /* Ignore this option, for backward compatibilitiy with
                   coreutils 5.92. */
                break;
            case 'f':
                x.interactive = RMI_NEVER;
                x.ignore_missing_files = true;
                prompt_once = false;
                break;
            case 'i':
                x.interactive = RMI_ALWAYS;
                x.ignore_missing_files = false;
                prompt_once = false;
                break;
            case 'I':
                x.interactive = RMI_NEVER;
                x.ignore_missing_files = false;
                prompt_once = true;
                break;
            case 'r':
            case 'R':
                x.recursive = true;
                break;
            case INTERACTIVE_OPTION:
            {
                int i;
                if(optarg)
                    i = XARGMATCH("--interactive", optarg, interactive_args,
                                    interactive_types);
                else
                    i = interactive_always;

                switch(i)
                {
                    case interactive_never:
                        x.interactive = RMI_NEVER;
                        prompt_once = false;
                        break;
                    case interactive_once:
                        x.interactive = RMI_SOMETIMES;
                        x.ignore_missing_files = false;
                        prompt_once = true;
                        break;
                    case interactive_always:
                        x.interactive = RMI_ALWAYS;
                        x.ignore_missing_files = false;
                        prompt_once = false;
                        break;
                }
                break;
            }

            case ONE_FILE_SYSTEM:
                x.one_file_system = true;
                break;
            case NO_PRESERVE_ROOT:
                preserve_root = false;
                break;
            case PRESERVE_ROOT:
                preserve_root = true;
                break;
            case PRESUME_INPUT_TTY_OPTION:
                x.stdin_tty = true;
                break;
            case 'v':
                x.verbose = true;
                break;

            case_GETOPT_HELP_CHAR;
            case_GETOPT_VERSION_CHAR(PROGROM_NAME, AUTHORS);
            default:
                diagnostic_leading_hyphen(argc, argv);
                usage(EXIT_FAILURE);
        }
    }

    if(argc <= optind)
    {
        if(x.ignore_missing_files)
            exit(EXIT_SUCCESS);
        else
        {
            error(0, 0, _("missing operand"));
            usage(EXIT_FAILURE);
        }
    }

    if(x.recursive && preserve_root)
    {
        static struct dev_ino dev_ino_buf;
        x.root_dev_ino = get_root_dev_ino(&dev_ino_buf);
        if(x.root_dev_ino == NULL)
            error(EXIT_FAILURE, errno, _("failed to get attribute of %s"),
                    quote("/"));
    }

    size_t n_files = argc - optind;
    char** file = argv + optind;

    if(prompt_once && (x.recursive || n_files > 3))
    {
        fprintf(stderr,
                (x.recursive
                 ? _("%s: remove all arguments recursively? ")
                 : _("%s: remove all arguments? ")),
                 program_name);
        if(!yesno())
            exit(EXIT_SUCCESS);
    }

    enum RM_status status = rm(file, &x);
    assert(VALID_STATUS(status));
    exit(status == RM_ERROR ? EXIT_FAILURE : EXIT_SUCCESS);
}


