/* GNU's users */

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>

#include "system.h"
#include "long-options.h"
#include "quote.h"
#include "closeout.h"
#include "version.h"
#include "readutmp.h"
#include "xalloc.h"

#define PROGRAM_NAME "users"
#define AUTHORS "Joseph Arceneaux & David MacKenzie"

static int userid_compare(const void* v_a, const void* v_b)
{
    char** a = (char**)v_a;
    char** b = (char**)v_b;
    return strcmp(*a, *b);
}

static void list_entries_users(size_t n, const STRUCT_UTMP *this)
{
    char** u = xnmalloc(n, sizeof(*u));
    size_t i;
    size_t n_entries = 0;

    while(n--)
    {
        if(IS_USER_PROCESS(this))
        {
            char* trimmed_name;

            trimmed_name = extract_trimmed_name(this);

            u[n_entries] = trimmed_name;
            ++n_entries;
        }
        this++;
    }

    qsort(u, n_entries, sizeof(u[0]), userid_compare);

    for(i = 0; i < n_entries; i++)
    {
        char c = (i < n_entries - 1 ? ' ' : '\n');
        fputs(u[i], stdout);
        putchar(c);
    }

    for(i = 0; i < n_entries; i++)
        free(u[i]);
    free(u);
}

/* Display a list of users on the systems, according to utmp file FILENAME.
   Use read_utmp OPTIONS to read FILENAME. */
static void users(const char* filename, int options)
{
    size_t n_users;
    STRUCT_UTMP* utmp_buf;

    //printf("filename: %s\n", filename);
    if(read_utmp(filename, &n_users, &utmp_buf, options) != 0)
        error(EXIT_FAILURE, errno, "%s", filename);

    list_entries_users(n_users, utmp_buf);

    free(utmp_buf);
}

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, _("Try `%s --help' for more information.\n"),
                program_name);
    else
    {
        printf(_("Usage: %s [OPTION]... [FILE]\n"), program_name);
        printf(_("\
Output who is currently logged in according to FILE.\n\
If FILE is not specified, use %s. %s as FILE is common.\n\
\n\
"),
                UTMP_FILE, WTMP_FILE);
        fputs(HELP_OPTION_DESCRIPTION, stdout);
        fputs(VERSION_OPTION_DESCRIPTION, stdout);
        emit_ancillary_info();
    }
    exit(status);
}

int main(int argc, char** argv)
{
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

    switch(argc - optind)
    {
        case 0:     /* users */
            users(UTMP_FILE, READ_UTMP_CHECK_PIDS);
            break;
        case 1:     /* users <utmp file> */
            users(argv[optind], 0);
            break;
        default:
            error(0, 0, _("extra operand %s"), quote(argv[optind + 1]));
            usage(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
