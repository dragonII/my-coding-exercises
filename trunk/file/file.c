/* main - parse arguments and handle options */

#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <string.h>

#include <stdio.h>

#include <wchar.h>

#include "file.h"
#include "magic_.h"

static char* progname;       /* used throughout */

static void usage(void);
static void docprint(const char*);
static void help(void);

static int  unwrap(struct magic_set*, const char*);
static int  process(struct magic_set*, const char*, int);
static struct magic_set* load(const char*, int);

static size_t file_mbswidth(const char* s);

#ifdef S_IFLNK
#define FILE_FLAGS "-bchikLlNnprsvz0"
#else
#define FILE_FLAGS "-bciklNnprsvz0"
#endif

#define USAGE       \
    "Usage: %s [" FILE_FLAGS \
    "] [--apple] [--mime-encoding] [--mime-type]\n" \
    "            [-e testname] [-F separator] [-f namefile] [-m magicfiles] "   \
    "file ...\n"    \
    "       %s -C [-m magicfiles]\n"    \
    "       %s [--help]\n"

static int      /* Global command-line options */
    bflag = 0,  /* brief output format */
    nopad = 0,  /* don't pad output */
    nobuffer = 0, /* don't buffer stdout */
    nulsep = 0;   /* append '\0' to the separator */

static const char* separator = ":";     /* default field separator */


static const struct option long_options[] =
{
#define OPT(shortname, longname, opt, doc)  \
    {longname, opt, NULL, shortname},
#define OPT_LONGONLY(longname, opt, doc)    \
    {longname, opt, NULL, 0},
#include "file_opts.h"
#undef OPT
#undef OPT_LONGONLY
    {NULL, 0, NULL, 0}
};
#define OPTSTRING       "bcCde:f:F:hiklLm:nNprsvz0"


static const struct
{
    const char* name;
    int value;
} nv[] =
{
    {"apptype",     MAGIC_NO_CHECK_APPTYPE},
    {"ascii",       MAGIC_NO_CHECK_ASCII},
    {"cdf",         MAGIC_NO_CHECK_CDF},
    {"compress",    MAGIC_NO_CHECK_COMPRESS},
    {"elf",         MAGIC_NO_CHECK_ELF},
    {"encoding",    MAGIC_NO_CHECK_ENCODING},
    {"soft",        MAGIC_NO_CHECK_SOFT},
    {"tar",         MAGIC_NO_CHECK_TAR},
    {"text",        MAGIC_NO_CHECK_TEXT},   /* synonym for ascii */
    {"tokens",      MAGIC_NO_CHECK_TOKENS}, /* OBSOLETE: ignore for backwards compatibilities */
};

static void
usage(void)
{
    (void)fprintf(stderr, USAGE, progname, progname, progname);
    exit(1);
}


static size_t file_mbswidth(const char* s)
{
    size_t bytesconsumed, old_n, n, width = 0;
    mbstate_t state;
    wchar_t nextchar;
    (void)memset(&state, 0, sizeof(mbstate_t));
    old_n = n = strlen(s);

    while(n > 0)
    {
        bytesconsumed = mbrtowc(&nextchar, s, n, &state);
        if(bytesconsumed == (size_t)(-1) ||
           bytesconsumed == (size_t)(-1))
        {
            /* Something went wrong, return something reasonable */
            return old_n;
        }
        if(s[0] == '\n')
        {
            /* Do what strlen() would do, so that caller
               is always right */
            width++;
        } else
            width += wcwidth(nextchar);

        s += bytesconsumed, n -= bytesconsumed;
    }
    return width;
}

int main(int argc, char** argv)
{
    int c;
    size_t i;
    int action = 0, didsomefiles = 0, errflg = 0;
    int flags = 0, e = 0;
    struct magic_set* magic = NULL;
    int longindex;
    const char* magicfile = NULL;

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
                break;
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
            magic = magic_open(flags | MAGIC_CHECK);
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


static struct magic_set*
load(const char* magicfile, int flags)
{
    struct magic_set* magic = magic_open(flags);
    if(magic == NULL)
    {
        (void)fprintf(stderr, "%s: %s\n", progname, strerror(errno));
        return NULL;
    }
    if(magic_load(magic, magicfile) == -1)
    {
        (void)fprintf(stderr, "%s: %s\n",
                    progname, magic_error(magic));
        magic_close(magic);
        return NULL;
    }
    return magic;
}


/* unwrap -- read a file of filenames, do each one */
static int
unwrap(struct magic_set* ms, const char* fn)
{
    FILE* f;
    size_t len;
    char* line = NULL;
    size_t llen = 0;
    int wid = 0, cwid;
    int e = 0;

    if(strcmp("-", fn) == 0)
    {
        f = stdin;
        wid = 1;
    } else
    {
        if((f = fopen(fn, "r")) == NULL)
        {
            (void)fprintf(stderr, "%s: Cannot open `%s' (%s).\n",
                            progname, fn, strerror(errno));
            return 1;
        }

        while((len = getline(&line, &llen, f)) > 0)
        {
            if(line[len - 1] == '\n')
                line[len - 1] = '\0';
            cwid = file_mbswidth(line);
            if(cwid > wid)
                wid = cwid;
        }

        rewind(f);
    }

    while((len = getline(&line, &llen, f)) > 0)
    {
        if(line[len - 1] == '\n')
            line[len - 1] = '\0';
        e |= process(ms, line, wid);
        if(nobuffer)
            (void)fflush(stdout);
    }

    free(line);
    (void)fclose(f);
    return e;
}


/* Called for each input file on the command line (or in a list of files) */
static int
process(struct magic_set* ms, const char* inname, int wid)
{
    const char* type;
    int std_in = strcmp(inname, "-") == 0;

    if(wid > 0 && !bflag)
    {
        (void)printf("%s", std_in ? "/dev/stdin" : inname);
        if(nulsep)
            (void)putc('\0', stdout);
        (void)printf("%s", separator);
        (void)printf("%*s ",
                (int)(nopad ? 0 : (wid - file_mbswidth(inname))), "");
    }

    type = magic_file(ms, std_in ? NULL : inname);
    if(type == NULL)
    {
        (void)printf("ERROR: %s\n", magic_error(ms));
        return 1;
    } else
    {
        (void)printf("%s\n", type);
        return 0;
    }
}


static void
help(void)
{
    (void)fputs(
"Usage: file [OPTION...] [FILE...]\n"
"Determine type of FILEs.\n"
"\n", stdout);
#define OPT(shortname, longname, opt, doc)      \
    fprintf(stdout, "   -%c, --" longname, shortname),  \
    docprint(doc);
#define OPT_LONGONLY(longname, opt, doc)    \
    fprintf(stdout, "        --" longname), \
    docprint(doc);
#include "file_opts.h"
#undef OPT
#undef OPT_LONGONLY
    fprintf(stdout, "\nReport bugs to http://xxx.xxx.xxx\n");
    exit(0);
}


static void
docprint(const char* opts)
{
    size_t i;
    int comma;
    char *sp, *p;

    p = strstr(opts, "%o");
    if(p == NULL)
    {
        fprintf(stdout, "%s", opts);
        return;
    }

    for(sp = p - 1; sp > opts && *sp == ' '; sp--)
        continue;

    fprintf(stdout, "%.*s", (int)(p - opts), opts);

    comma = 0;
    for(i = 0; i < __arraycount(nv); i++)
    {
        fprintf(stdout, "%s%s", comma++ ? ", " : "", nv[i].name);
        if(i && i % 5 == 0)
        {
            fprintf(stdout, ",\n%*s", (int)(p - sp - 1), "");
            comma = 0;
        }
    }

    fprintf(stdout, "%s", opts + (p - opts) + 2);
}
