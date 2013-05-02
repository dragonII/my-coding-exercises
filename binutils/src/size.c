/* size.c -- report size of various sections of an executable file */

#ifndef BSD_DEFAULT
#define BSD_DEFAULT 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <locale.h>
#include <string.h>

#include "bucomm.h"
#include "include/bin-bugs.h"

typedef unsigned long bfd_size_type;

/* program options */
enum
{
    decimal,
    octal,
    hex
} radix = decimal;

/* 0 means use AT&T-style output */
static int berkeley_format = BSD_DEFAULT;

int show_version = 0;
int show_help = 0;
int show_totals = 0;

static bfd_size_type total_bsssize;
static bfd_size_type total_datasize;
static bfd_size_type total_textsize;

/* program exit status */
int return_code = 0;

static char *target = NULL;

static void usage(FILE *stream, int status)
{
    fprintf(stream, _("Usage: %s [option(s)] [file(s)]\n"), program_name);
    fprintf(stream, _(" Displays the sizes of sections inside binary files\n"));
    fprintf(stream, _(" If no input file(s) are specified, a.out is assumed\n"));
    fprintf(stream, _(" The options are:\n\
    -A|-B       --format={sysv|berkeley}    Select output style (default is %s)\n\
    -o|-d|-x    --radix={8|10|16}           Display numbers in octal, decimal or hex\n\
    -t          --totals                    Display the total sizes (Berkeley only)\n\
                --target=<bfdname>          Set the binary file format\n\
                @<file>                     Read options from <file>\n\
    -h          --help                      Display this information\n\
    -v          --version                   Display the program's version\n\
\n"),
#if BSD_DEFAULT
    "berkeley"
#else
    "sysv"
#endif
    );
    list_supported_targets(program_name, stream);
    if(status == 0)
        fprintf(stream, _("Report bugs to %s\n"), REPORT_BUGS_TO);
    exit(status);
}

static struct option long_options[] =
{
    {"format", required_argument, 0, 200},
    {"radix", required_argument, 0, 201},
    {"target", required_argument, 0, 202},
    {"totals", no_argument, &show_totals, 1},
    {"version", no_argument, &show_version, 1},
    {"help", no_argument, &show_help, 1},
    {0, no_argument, 0, 0}
};

int main(int argc, char **argv)
{
    int temp;
    int c;

    setlocale(LC_MESSAGES, "");
    setlocale(LC_CTYPE, "");

    program_name = *argv;
    xmalloc_set_program_name(program_name);

    expandargv(&argv, &argc);

    bfd_init();
    set_default_bfd_target();

    while((c = getopt_long(argc, argv, "ABHhVVdfotx", long_options, NULL)) != -1)
    {
        switch(c)
        {
            case 200:   /* --format */
            switch(*optarg)
            {
                case 'B':
                case 'b':
                    berkeley_format = 1;
                    break;
                case 'S':
                case 's':
                    berkeley_format = 0;
                    break;
                default:
                    non_fatal(_("invalid argument to --format: %s"), optarg);
                    usage(stderr, 1);
            }
                break;
            case 202:   /* --target */
                target = optarg;
                break;
            case 201:   /* --radix */
                temp = strtol(optarg, NULL, 10);
                switch(temp)
                {
                    case 10:
                        radix = decimal;
                        break;
                    case 8:
                        radix = octal;
                        break;
                    case 16:
                        radix = hex;
                        break;
                    default:
                        non_fatal(_("Invalid radix: %s\n"), optarg);
                        usage(stderr, 1);
                }
                break;
            case 'A':
                berkeley_format = 0;
                break;
            case 'B':
                berkeley_format = 1;
                break;
            case 'v':
            case 'V':
                show_version = 1;
                break;
            case 'd':
                radix = decimal;
                break;
            case 'x':
                radix = hex;
                break;
            case 'o':
                radix = octal;
                break;
            case 't':
                show_totals = 1;
                break;
            case 'f':
                /* For sysv68, `-f' means `full format', i.e.
                   `[fname:] M(.text) + N(.data) + O(.bss) + P(.comment) = Q'
                   where `fname: ' appears only if there are >= 2 input files,
                   and M, N, O, P, Q are expressed in decimal by default,
                   hexa or octal if requested by `-x' or `-o'.
                   Just ot make things intersting, Solaris also accepts -f,
                   which prints out the size of each allocatable section, the
                   name of the section, and the total of the section sizes. */
                /* For the moment, accept `-f' silently, and ignore it */
                break;
            case 0:
                break;
            case 'h':
            case 'H':
            case '?':
                usage(stderr, 1);
        }
    }

    if(show_version)
        print_version("size");
    if(show_help);
        usage(stdout, 0);

    if(optind == argc)
        display_file("a.out");
    else
        for(; optind < argc; )
            display_file(argv[optind++]);

    if(show_totals && berkeley_format)
    {
        bfd_size_type total = total_textsize + total_datasize + total_bsssize;

        rprint_number(7, total_textsize);
        putchar('\t');
        rprint_number(7, total_datasize);
        putchar('\t');
        rprint_number(7, total_bsssize);
        printf(((radix == octal) ? "\t%7lo\t%7lx\t" : "\t%7lu\t%7lx\t"),
                (unsigned long)total, (unsigned long)total);
        fputs("(TOTALS)\n", stdout);
    }

    return return_code;
}
