#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <error.h>
#include <errno.h>

/* If true, do not output the trailing newline */
static bool no_newline;

/* If true, report error messages */
static bool verbose;

static struct option longopts[] =
{
    {"canonicalize", no_argument, NULL, 'f'},
    {"canonicalize-existing", no_argument, NULL, 'e'},
    {"canonicalize-missing", no_argument, NULL, 'm'},
    {"no-newline", no_argument, NULL, 'n'},
    {"quiet", no_argument, NULL, 'q'},
    {"silent", no_argument, NULL, 's'},
    {"verbose", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
};

//static struct option longopts[] =
//{
//    {"adjustment", required_argument, NULL, 'n'},
//    {NULL, 0, NULL, 0}
//};


void usage(int status)
{
    printf("Error and exit\n");
    exit(status);
}

#define ISDIGIT(c) ((unsigned int)(c) - '0' <= 9)

//int main(int argc, char** argv)
//{
//    int i;
//    char* adjustment_given;
//    for(i = 1; i < argc; /* empty */)
//    {
//        char* s = argv[i];
//
//        if(s[0] == '-' && ISDIGIT(s[1 + (s[1] == '-' || s[1] == '+')]))
//        {
//            adjustment_given = s + 1;
//            ++i;
//        }
//        else
//        {
//            int optc;
//            int fake_argc = argc - (i - 1);
//            char** fake_argv = argv + (i - 1);
//
//            fake_argv[0] = argv[0];
//
//            optind = 0;
//
//            optc = getopt_long(fake_argc, fake_argv, "+n:", longopts, NULL);
//            printf("optc = %d, i = %d, optind = %d, optarg = %s\n", optc, i, optind, optarg);
//            i += optind - 1;
//
//            if(optc == '?')
//            {
//                printf("missing option argument\n");
//                return -1;
//            }
//            else if(optc == 'n')
//                adjustment_given = optarg;
//            else /* optc == -1 */
//                break;
//        }
//    }
//    if(adjustment_given)
//    {
//        printf("adjustment_given: %s\n", adjustment_given);
//        return 0;
//    }
//    return 1;
//}

int main(int argc, char** argv)
{
    int can_mode = -1;

    const char* fname;

    int optc;

    while((optc = getopt_long(argc, argv, "efmnqsv", longopts, NULL)) != -1)
    {
        switch(optc)
        {
            case 'e':
                can_mode = 'e';
                printf("[E] argv[optind] = %s\n", argv[optind]);
                printf("[E] optind = %d\n", optind);
                break;
            case 'f':
                can_mode = 'f';
                printf("[F] optind = %d\n", optind);
                break;
            case 'm':
                can_mode = 'm';
                printf("[M] optind = %d\n", optind);
                break;
            case 'n':
                no_newline = true;
                printf("[N] optind = %d\n", optind);
                break;
            case 'q':
            case 's':
                verbose = false;
                printf("[S/Q] optind = %d\n", optind);
                break;
            case 'v':
                verbose = true;
                printf("[V] optind = %d\n", optind);
                break;
            default:
                printf("wrong options %c\n", optc);
                printf("[Wrong] optind = %d\n", optind);
                return -1;
        }
    }

    printf("[MAIN1] optind = %d\n", optind);
    if(optind >= argc)
    {
        error(0, 0, "missing operand");
        printf("[ERROR] optind = %d\n", optind);
        usage(EXIT_FAILURE);
    }

    fname = argv[optind++];
    printf("[MAIN2] optind = %d\n", optind);

    if(optind < argc)
    {
        error(0, 0, "extra operand %s", argv[optind]);
        usage(EXIT_FAILURE);
    }

    printf("e: %c\n", can_mode);
    printf("f: %c\n", can_mode);
    printf("m: %c\n", can_mode);
    printf("no_newline: %s\n", no_newline ? "true" : "false");
    printf("verbose: %s\n", verbose ? "true" : "false");
    printf("filename: %s\n", fname);

    return EXIT_FAILURE;
}
