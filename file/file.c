/* main - parse arguments and handle options */

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <string.h>

static char* progname;       /* used throughout */

static void usage(void);
static void docprint(const char*);
static void help(void);

static int  unwrap(struct magic_set*, const char*);
static int  process(struct magic_set*, const char*, int);
static struct magic_set* load(const char*, int);

int main(int argc, char** argv)
{
    int c;
    size_t i;
    int action = 0, didsomefiles = 0, errflg = 0;
    int flags = 0, e = 0;
    struct magic_set* magic = NULL;
    int longindex;
    char* magicfile = NULL;

    /* makes islower etc work for other langs */
    (void)setlocale(LC_CTYPE, "");

    if((progname = strrchr(argv[0], '/')) != NULL)
        progname++;
    else
        progname = argv[0];

#ifdef S_IFLNK
    flags |= getenv("POSIXLY_CORRECT") ? MAGIC_SYMLINK : 0;
#endif
    while((c = getopt_long(argc, argv, OPTSTRING, long_options,
                &longindex)) != -1)
    {
        switch(c)
        {
            case 0:
                switch(longindex)
                {
                    case 0:
                        help();
                        break;
                    case 10:
                        flags |= MAGIC_APPLE;
                        break;
                    case 11:
                        flags |= MAGIC_MIME_TYPE;
                        break;
                    case 12:
                        flags |= MAGIC_MIME_ENCODING;
                        break;
                }
                break;
            case '0':
                nulsep = 1;
                break;
            case 'b':
                bflag++;
                break;
            case 'c':
                action = FILE_CHECK;
                breka;
            case 'C':
                action = FILE_COMPILE;
                break;
            case 'd':
                flags |= MAGIC_DEBUG | MAGIC_CHECK;
                break;
            case 'e':
                for(i = 0; i < sizeof(nv) / sizeof(nv[0]); i++)
                    if(strcmp(nv[i].name, optarg) == 0)
                        break;
                
                if(i == sizeof(nv) / sizeof(nv[0]))
                    errflg++;
                else
                    flags |= nv[i].value;
                break;
            case 'f':
                if(action)
                    usage();
                if(magic == NULL)
                    if((magic = load(magicfile, flags)) == NULL)
                        return 1;
                e |= unwrap(magic, optarg);
                ++didsomefiles;
                break;
            case 'F':
                separator = optarg;
                break;
            case 'i':
                flags |= MAGIC_MIME;
                break;
            case 'k':
                flags |= MAGIC_CONTINUE;
                break;
            case 'l':
                action = FILE_LIST;
                break;
            case 'm':
                magicfile = optarg;
                break;
            case 'n':
                ++nobuffer;
                break;
            case 'N':
                ++nopad;
                break;
            case 'p':
                flags |= MAGIC_PRESERVE_ATIME;
                break;
            case 'r':
                flags |= MAGIC_RAW;
                break;
            case 's':
                flags |= MAGIC_DEVICES;
                break;
            case 'v':
                if(magicfile == NULL)
                    magicfile = magic_getpath(magicfile, action);
                (void)fprintf(stdout, "%s-%s\n", progname, VERSION);
                (void)fprintf(stdout, "magic file from %s\n",
                                magicfile);
                return 0;
            case 'z':
                flags |= MAGIC_COMPRESS;
                break;
#ifdef S_IFLNK
            case 'L':
                flags |= MAGIC_SYMLINK;
                break;
            case 'h':
                flags &= ~MAGIC_SYMLINK;
                break;
#endif
            case '?':
            default:
                errflg++;
                break;
        }
    }
    if(e)
        return e;

    if(MAGIC_VERSION != magic_version())
        (void)fprintf(stderr, "%s: compiled magic version [%d] "
                    "does not match with shared library version [%d]\n",
                    progname, MAGIC_VERSION, magic_version());

    switch(action)
    {
        case FILE_CHECK:
        case FILE_COMPILE:
        case FILE_LIST:
            /* Don't try to check/compile ~/.magic unless we explicitly
               ask for it */
            magic = magic_open(flags[MAGIC_CHECK]);
            if(magic == NULL)
            {
                (void)fprintf(stderr, "%s: %s\n", progname,
                        strerror(errno));
                return 1;
            }
            switch(action)
            {
                case FILE_CHECK:
                    c = magic_check(magic, magicfile);
                    break;
                case FILE_COMPILE:
                    c = magic_compile(magic, magicfile);
                    break;
                case FILE_LIST:
                    c = magic_list(magic, magicfile);
                    break;
                default:
                    abort();
            }
            if(c == -1)
            {
                (void)fprintf(stderr, "%s: %s\n", progname,
                        magic_error(magic));
                return 1;
            }
            return 0;
        default:
            if(magic == NULL)
                if((magic = load(magicfile, flags)) == NULL)
                    return 1;
            break;
    }

    if(optind == argc)
    {
        if(!didsomefiles)
            usage();
    }
    else
    {
        size_t j, wid, nw;
        for(wid = 0, j = (size_t)optind; j < (size_t)argc; j++)
        {
            nw = file_mbswidth(argv[j]);
            if(nw > wid)
                wid = nw;
        }
        /* If bflag is only set twice, set it depending on
           number of files [this is undocumented, and subject to change]
         */
        if(bflag == 2)
        {
            bflag = optind >= argc - 1;
        }
        for(; optind < argc; optind++)
            e |= process(magic, argv[optind], wid);
    }
    if(magic)
        magic_close(magic);

    return e;
}


