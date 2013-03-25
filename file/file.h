/* file.h - definitions for file(1) program */

#ifndef __file_h__
#define __file_h__

#include <stdint.h>

#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#endif

#ifndef __arraycount
#define __arraycount(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef HOWMANY
#define HOWMANY (256 * 1024)    /* how much of the file to look at */
#endif

#define MAXMAGIC 8192   /* max entries in any one magic file or directory */

#define ENABLE_CONDITIONALS

#ifndef MAGIC
#define MAGIC "/etc/magic"
#endif

#define MAXDESC     64  /* max len of text description/MIME type */
#define MAXMIME     80  /* max len of text MIME type */
#define MAXstring   64  /* max len of "string" types */

#define FILE_LOAD       0
#define FILE_CHECK      1
#define FILE_COMPILE    2
#define FILE_LIST       3

union VALUETYPE
{
    uint8_t     b;
    uint16_t    h;
    uint32_t    l;
    uint64_t    q;
    uint8_t     hs[2];      /* 2 bytes of a fixed-endian "short" */
    uint8_t     hl[4];      /* 4 bytes of a fixed-endian "long"  */
    uint8_t     hq[8];      /* 8 bytes of a fixed-endian "quad"  */
    char s[MAXstring];      /* the search string or regex pattern */
    unsigned char us[MAXstring];
    float       f;
    double      d;
};


struct magic
{
    /* Word 1 */
    uint16_t    cont_level;     /* level of ">" */
    uint8_t     flag;
#define INDIR           0x01    /* if '(...)' appears */
#define OFFADD          0x02    /* if '>&' or '>...(&' appears */
#define INDIROFFADD     0x04    /* if '>&(' appears */
#define UNSIGNED        0x08    /* comparison is unsigned */
#define NOSPACE         0x10    /* suppress space character before output */
#define BINTEST         0x20    /* test if for a binary type (set only for toplevel tests) */
#define TEXTTEST        0x40    /* for passing to file_softmagic */

    uint8_t     factor;

    /* Word 2 */
    uint8_t     reln;           /* relation (0=eq, '>'=gt, etc) */
    uint8_t     vallen;         /* length of string value, if any */
    uint8_t     type;           /* comparison type (FILE_*) */
    uint8_t     in_type;        /* type of indirection */

#define FILE_INVALI     0
#define FILE_BYTE       1
#define FILE_SHORT      2
#define FILE_DEFAULT    3
#define FILE_LONG       4
#define FILE_STRING     5
#define FILE_DATE       6
#define FILE_BESHORT    7
#define FILE_BELONG     8
#define FILE_BEDATE     9
#define FILE_LESHORT    10
#define FILE_LELONG     11
#define FILE_LEDATE     12
#define FILE_PSTRING    13
#define FILE_LDATE      14
#define FILE_BELDATE    15
#define FILE_LELDATE    16
#define FILE_REGEX      17
#define FILE_BESTRING16	18
#define FILE_LESTRING16	19
#define FILE_SEARCH     20
#define FILE_MEDATE	    21
#define FILE_MELDATE    22
#define FILE_MELONG	    23
#define FILE_QUAD	    24
#define FILE_LEQUAD	    25
#define FILE_BEQUAD	    26
#define FILE_QDATE	    27
#define FILE_LEQDATE    28
#define FILE_BEQDATE    29
#define FILE_QLDATE	    30
#define FILE_LEQLDATE   31
#define FILE_BEQLDATE   32
#define FILE_FLOAT	    33
#define FILE_BEFLOAT    34
#define FILE_LEFLOAT    35
#define FILE_DOUBLE	    36
#define FILE_BEDOUBLE   37
#define FILE_LEDOUBLE   38
#define FILE_BEID3	    39
#define FILE_LEID3	    40
#define FILE_INDIRECT   41
#define FILE_QWDATE	    42
#define FILE_LEQWDATE   43
#define FILE_BEQWDATE   44
#define FILE_NAME       45
#define FILE_USE        46
#define FILE_NAMES_SIZE 47 /* size of array to contain all names */

#define IS_STRING(t)            \
    ((t) == FILE_STRING     ||  \
     (t) == FILE_PSTRING    ||  \
     (t) == FILE_BESTRING16 ||  \
     (t) == FILE_LESTRING16 ||  \
     (t) == FILE_REGEX      ||  \
     (t) == FILE_SEARCH     ||  \
     (t) == FILE_NAME       ||  \
     (t) == FILE_USE        ||  \
     (t) == FILE_DEFAULT)

#define FILE_FMT_NONE       0
#define FILE_FMT_NUM        1   /* "cduxXi" */
#define FILE_FMT_STR        2   /* "s" */
#define FILE_FMT_QUAD       3   /* "ll" */
#define FILE_FMT_FLOAT      4   /* "eEfFgG" */
#define FILE_FMT_DOUBLE     5   /* "eEfFgG" */

    /* Word 3 */
    uint8_t in_op;          /* operator for indirection */
    uint8_t mask_op;        /* operator for mask */
#ifdef ENABLE_CONDITIONALS
    uint8_t cond;           /* conditional type */
#else
    uint8_t dummy;
#endif
    uint8_t factor_op;


#define FILE_FACTOR_OP_PLUS	    '+'
#define FILE_FACTOR_OP_MINUS    '-'
#define FILE_FACTOR_OP_TIMES    '*'
#define FILE_FACTOR_OP_DIV      '/'
#define FILE_FACTOR_OP_NONE     '\0'

#define FILE_OPS            "&|^+-*/%"
#define FILE_OPAND          0
#define FILE_OPOR           1
#define FILE_OPXOR          2
#define FILE_OPADD          3
#define FILE_OPMINUS        4
#define FILE_OPMULTIPLY     5
#define FILE_OPDIVIDE       6
#define FILE_OPMODULO       7
#define FILE_OPS_MASK       0x07 /* mask for above ops */
#define FILE_UNUSED_1       0x08
#define FILE_UNUSED_2       0x10
#define FILE_UNUSED_3       0x20
#define FILE_OPINVERSE      0x40
#define FILE_OPINDIRECT     0x80

#ifdef ENABLE_CONDITIONALS
#define COND_NONE   0
#define COND_IF	    1
#define COND_ELIF   2
#define COND_ELSE   3
#endif /* ENABLE_CONDITIONALS */

    /* Word 4 */
    uint32_t offset;        /* offset to magic number */
    /* Word 5 */
    uint32_t in_offset;     /* offset from indirection */
    /* Word 6 */
    uint32_t lineno;        /* line number in magic file */
    /* Word 7,8 */
    union
    {
        uint64_t _mask;     /* for use with numeric and date types */
        struct 
        {
            uint32_t _count;    /* repeat/line count */
            uint32_t _flags;    /* modifier flags */
        } _s;               /* for use with string types */
    } _u;
#define num_mask    _u._mask
#define str_range   _u._s._count
#define str_flags   _u._s._flags
    /* Words 9-16 */
    union VALUETYPE value;      /* either number or string */
    /* Words 17-32 */
    char desc[MAXDESC];         /* description */
    /* Words 33-52 */
    char mimetype[MAXMIME];     /* MIME type */
    /* Words 53-54 */
    char apple[8];
};



/* list of magic entries */
struct mlist
{
    struct magic *magic;        /* array of magic entries */
    uint32_t      nmagic;       /* number of entries in array */
    void* map;                  /* internal resources used by entry */
    struct mlist *next, *prev;
};


#define MAGIC_SETS 2

struct magic_set
{
    struct mlist* mlist[MAGIC_SETS];        /* list of regular entries */
    struct cont
    {
        size_t len;
        struct level_info* li;
    } c;
    struct out
    {
        char* buf;                          /* Accumulation buffer */
        char* pbuf;                         /* Pritable buffer */
    } o;
    uint32_t offset;
    int error;
    int flags;                              /* Control magic tests */
    int event_flags;                        /* Note things that happened */
#define EVENT_HAD_ERR   0x01
    const char* file;
    size_t line;                            /* current magic line number */

    /* data for searches */
    struct
    {
        const char* s;                      /* start of search in original source */
        size_t s_len;                       /* length of search region */
        size_t offset;                      /* starting offset in source */
        size_t rm_len;                      /* match length */
    } search;

    /* Make the string dynamically allocated so that e.g.
       strings matched in files can be longer than MAXstring */
    union VALUETYPE ms_value;               /* either number or string */
};



#endif
