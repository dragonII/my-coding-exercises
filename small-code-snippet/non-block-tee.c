/* non-block-non - read from standard input and write to standard output and files */

#include <sys/types.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <assert.h>

#define PROGRAM_NAME "nb_tee"

#ifndef O_BINARY
#define O_BINARY    0x0000
#endif

#ifndef STREQ
#define STREQ(a, b) (strcmp ((a), (b)) == 0)
#endif

static bool append;
static bool ignore_interrupts;

static struct option const long_options[] =
{
    {"append", no_argument, NULL, 'a'},
    {"ignore-interrupts", no_argument, NULL, 'i'},
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
};

void usage(int status)
{
    if(status != EXIT_SUCCESS)
        fprintf(stderr, "Try '%s --help' for more information.\n",
            PROGRAM_NAME);
    else
    {
        printf("Usage: %s [OPTION]... [FILE]...\n", PROGRAM_NAME);
    }
    exit(status);
}


static bool
nb_tee_files(int nfiles, const char **files)
{
    FILE **descriptors;
    char buffer[BUFSIZ];
    ssize_t bytes_read;
    int i;
    bool ok = true;
    char *mode_string = 
            (O_BINARY
             ? (append ? "ab" : "wb")
             : (append ? "a" : "w"));

    assert(nfiles == 1);

    descriptors = (FILE **)malloc((nfiles + 1) * sizeof(*descriptors));

    for(i = nfiles; i >= 1; i--)
        files[i] = files[i - 1];

    if(O_BINARY && !isatty(STDIN_FILENO))
        freopen(NULL, "rb", stdin);
    if(O_BINARY && !isatty(STDOUT_FILENO))
        freopen(NULL, "wb", stdout);

    descriptors[0] = stdout;
    files[0] = "standard output";
    setvbuf(stdout, NULL, _IONBF, 0);

    for(i = 1; i <= nfiles; i++)
    {
        descriptors[i] = (STREQ(files[i], "-")
                            ? stdout
                            : fopen(files[i], mode_string));
        if(descriptors[i] == NULL)
        {
            error(0, errno, "%s", files[i]);
            ok = false;
        } else
            setvbuf(descriptors[i], NULL, _IONBF, 0);
    }

    while(1)
    {
        bytes_read = read(0, buffer, sizeof(buffer));
#ifdef EINTR
        if(bytes_read < 0 && errno == EINTR)
            continue;
#endif
        if(bytes_read <= 0)
            break;

        /* Write to all NFILES + 1 descriptors.
           Standard output is the first one */
        for(i = 0; i <= nfiles; i++)
            if(descriptors[i] //&& fflush(descriptors[i])
                && fwrite(buffer, bytes_read, 1, descriptors[i]) != 1)
            {
                error(0, errno, "%s", files[i]);
                descriptors[i] = NULL;
                ok = false;
            }
    }

    if(bytes_read == -1)
    {
        error(0, errno, "read error");
        ok = false;
    }

    /* Close the files, but not standard output */
    for(i = 1; i <= nfiles; i++)
        if(!STREQ(files[i], "-")
            && descriptors[i] && fclose(descriptors[i]) != 0)
        {
            error(0, errno, "%s", files[i]);
            ok = false;
        }

    free(descriptors);

    return ok;
}


int main(int argc, char **argv)
{
    bool ok;
    int optc;

    while((optc = getopt_long(argc, argv, "ai", long_options, NULL)) != -1)
    {
        switch(optc)
        {
            case 'a':
                append = true;
                break;
            case 'i':
                ignore_interrupts = true;
                break;
            case 'h':
                usage(EXIT_SUCCESS);
                break;
            case 'v':
                usage(EXIT_SUCCESS);
                break;
            default:
                usage(EXIT_FAILURE);
        }
    }

    if(ignore_interrupts)
        signal(SIGINT, SIG_IGN);

    ok = nb_tee_files(argc - optind, (const char **)&argv[optind]);
    if(close(STDIN_FILENO) != 0)
        error(EXIT_FAILURE, errno, "standard input");

    exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
}

