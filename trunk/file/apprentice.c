#include "file.h"
#include "magic_.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>


#define EATAB {while (isascii((unsigned char) *l) && \
                isspace((unsigned char) *l)) ++l; }


#define ALLOC_CHUNK (size_t)10

int file_formats[FILE_NAMES_SIZE];
const char* file_names[FILE_NAMES_SIZE];

static size_t magicsize = sizeof(struct magic);

struct magic_entry
{
    struct magic* mp;
    uint32_t cont_count;
    uint32_t max_count;
};

struct magic_map
{
    void* p;
    size_t len;
    struct magic* magic[MAGIC_SETS];
    uint32_t nmagic[MAGIC_SETS];
};


static int parse_mime(struct magic_set*, struct magic_entry*, const char*);
static int parse_strength(struct magic_set*, struct magic_entry*, const char*);
static int parse_apple(struct magic_set*, struct magic_entry*, const char*);

static struct 
{
    const char* name;
    size_t len;
    int (*fun)(struct magic_set*, struct magic_entry*, const char*);
} bang[] =
{
#define DECLARE_FIELD(name) { # name, sizeof(# name) - 1, parse_ ## name }
    DECLARE_FIELD(mime),
    DECLARE_FIELD(apple),
    DECLARE_FIELD(strength),
#undef DECLARE_FIELD
    { NULL, 0, NULL }
};


struct type_tbl_s
{
    const char name[16];
    const size_t len;
    const int  type;
    const int  format;
};


static const char usg_hdr[] = "cont\toffset\ttype\topcode\tmask\tvalue\tdesc";


/* XXX - the actual Single UNIX Specification says that "long" means "long",
   as in the C data type, but we treat it as meaning "4-byte integer".
   Given that the OS X version of file 5.04 did the same, I guess that passes
   the actual test; having "long" be dependent on how big a "long" is on
   the machine running "file" is silly.
 */
static const struct type_tbl_s type_tbl[] =
{
#define XX(s)       s, (sizeof(s) - 1)
#define XX_NULL     "", 0
    { XX("invalid"),    FILE_INVALID,       FILE_FMT_NONE },
    { XX("byte"),       FILE_BYTE,          FILE_FMT_NUM },
    { XX("short"),      FILE_SHORT,         FILE_FMT_NUM },
    { XX("default"),    FILE_DEFAULT,       FILE_FMT_STR },
    { XX("long"),       FILE_LONG,          FILE_FMT_NUM },
    { XX("string"),     FILE_STRING,        FILE_FMT_STR },
    { XX("date"),       FILE_DATE,          FILE_FMT_STR },
    { XX("beshort"),    FILE_BESHORT,       FILE_FMT_NUM },
    { XX("belong"),     FILE_BELONG,        FILE_FMT_NUM },
    { XX("bedate"),     FILE_BEDATE,        FILE_FMT_STR },
    { XX("leshort"),    FILE_LESHORT,       FILE_FMT_NUM },
    { XX("lelong"),     FILE_LELONG,        FILE_FMT_NUM },
    { XX("ledate"),     FILE_LEDATE,        FILE_FMT_STR },
    { XX("pstring"),    FILE_PSTRING,       FILE_FMT_STR },
    { XX("ldate"),      FILE_LDATE,         FILE_FMT_STR },
    { XX("beldate"),    FILE_BELDATE,       FILE_FMT_STR },
    { XX("leldate"),    FILE_LELDATE,       FILE_FMT_STR },
    { XX("regex"),      FILE_REGEX,         FILE_FMT_STR },
    { XX("bestring16"), FILE_BESTRING16,    FILE_FMT_STR },
    { XX("lestring16"), FILE_LESTRING16,    FILE_FMT_STR },
    { XX("search"),     FILE_SEARCH,        FILE_FMT_STR },
    { XX("medate"),     FILE_MEDATE,        FILE_FMT_STR },
    { XX("meldate"),    FILE_MELDATE,       FILE_FMT_STR },
    { XX("melong"),     FILE_MELONG,        FILE_FMT_NUM },
    { XX("quad"),       FILE_QUAD,          FILE_FMT_QUAD },
    { XX("lequad"),     FILE_LEQUAD,        FILE_FMT_QUAD },
    { XX("bequad"),     FILE_BEQUAD,        FILE_FMT_QUAD },
    { XX("qdate"),      FILE_QDATE,         FILE_FMT_STR },
    { XX("leqdate"),    FILE_LEQDATE,       FILE_FMT_STR },
    { XX("beqdate"),    FILE_BEQDATE,       FILE_FMT_STR },
    { XX("qldate"),     FILE_QLDATE,        FILE_FMT_STR },
    { XX("leqldate"),   FILE_LEQLDATE,      FILE_FMT_STR },
    { XX("beqldate"),   FILE_BEQLDATE,      FILE_FMT_STR },
    { XX("float"),      FILE_FLOAT,         FILE_FMT_FLOAT },
    { XX("befloat"),    FILE_BEFLOAT,       FILE_FMT_FLOAT },
    { XX("lefloat"),    FILE_LEFLOAT,       FILE_FMT_FLOAT },
    { XX("double"),     FILE_DOUBLE,        FILE_FMT_DOUBLE },
    { XX("bedouble"),   FILE_BEDOUBLE,      FILE_FMT_DOUBLE },
    { XX("ledouble"),   FILE_LEDOUBLE,      FILE_FMT_DOUBLE },
    { XX("leid3"),      FILE_LEID3,         FILE_FMT_NUM },
    { XX("beid3"),      FILE_BEID3,         FILE_FMT_NUM },
    { XX("indirect"),   FILE_INDIRECT,      FILE_FMT_NUM },
    { XX("qwdate"),     FILE_QWDATE,        FILE_FMT_STR },
    { XX("leqwdate"),   FILE_LEQWDATE,      FILE_FMT_STR },
    { XX("beqwdate"),   FILE_BEQWDATE,      FILE_FMT_STR },
    { XX("name"),       FILE_NAME,          FILE_FMT_NONE },
    { XX("use"),        FILE_USE,           FILE_FMT_NONE },
    { XX_NULL,          FILE_INVALID,       FILE_FMT_NONE },
};

/* There are not types, and cannot be preceded by "u" to make them
   unsigned */
static const struct type_tbl_s special_tbl[] =
{
    { XX("name"),   FILE_NAME,     FILE_FMT_STR },
    { XX("use"),    FILE_USE,      FILE_FMT_STR },
    { XX_NULL,      FILE_INVALID,  FILE_FMT_NONE },
};
#undef XX
#undef XX_NULL

static int 
get_type(const struct type_tbl_s* tbl, const char* l, const char** t)
{
    const struct type_tbl_s* p;
    for(p = tbl; p->len; p++)
    {
        if(strncmp(l, p->name, p->len) == 0)
        {
            if(t)
                *t = l + p->len;
            break;
        }
    }
    return p->type;
}


static int
get_standard_integer_type(const char* l, const char** t)
{
    int type;

    if(isalpha((unsigned char)l[1]))
    {
        switch(l[1])
        {
            case 'C':
                /* "dC" and "uC" */
                type = FILE_BYTE;
                break;
            case 'S':
                /* "dS" and "uS" */
                type = FILE_SHORT;
                break;
            case 'I':
            case 'L':
                /* "dI", "dL", "uI", and "uL" */
                
                /* The actual Single UNIX Specification says
                   that "L" means "long", as in the C data type,
                   but we treat it as meaning "4-type integer".
                   Given that the OS X version of file 5.04 did
                   the same, I guess that passes the actual SUS
                   validation suite; having "dL" be dependent on
                   how big a "long" is on the machine running
                   "file" is silly */
                type = FILE_LONG;
                break;
            case 'Q':
                /* "dQ" and "uQ" */
                type = FILE_QUAD;
                break;
            default:
                /* "d{anything else}", "u{anything else}" */
                return FILE_INVALID;
        }
        l += 2;
    } else if(isdigit((unsigned char)l[1]))
    {
        /* "d{num}" and "u{num}"; we only support {num} values
           of 1, 2, 4, and 8 - the Single UNIX Specification
           doesn't say anything about whether arbitrary
           values should be supported, but both the Solaris 10
           and OS X Mountain Lion versions of file passed the
           Single UNIX Specification validation suite, and
           neither of them support values bigger than 8 or
           non-power-of-2 values */
        if(isdigit((unsigned char)l[2]))
        {
            /* Multi-digit, so > 9 */
            return FILE_INVALID;
        }
        switch(l[1])
        {
            case '1':
                type = FILE_BYTE;
                break;
            case '2':
                type = FILE_SHORT;
                break;
            case '4':
                type = FILE_LONG;
                break;
            case '8':
                type = FILE_QUAD;
                break;
            default:
                return FILE_INVALID;
        }
        l += 2;
    } else
    {
        /* "d" or "u" by itself */
        type = FILE_LONG;
        ++l;
    }
    if(t)
        *t = l;

    return type;
}


static void
init_file_tables(void)
{
    static int done = 0;
    const struct type_tbl_s* p;

    if(done)
        return;
    done++;

    for(p = type_tbl; p->len; p++)
    {
        assert(p->type < FILE_NAMES_SIZE);
        file_names[p->type] = p->name;
        file_formats[p->type] = p->format;
    }
    assert(p - type_tbl == FILE_NAMES_SIZE);
}


/* Parse a file or directory of files
   const char* fn: name of magic file or directory */
static int
cmpstrp(const void* p1, const void* p2)
{
    return strcmp(*(char* const*)p1, *(char* const*)p2);
}


static int get_op(char c)
{
    switch(c)
    {
        case '&':
            return FILE_OPAND;
        case '|':
            return FILE_OPOR;
        case '^':
            return FILE_OPXOR;
        case '-':
            return FILE_OPMINUS;
        case '+':
            return FILE_OPADD;
        case '*':
            return FILE_OPMULTIPLY;
        case '/':
            return FILE_OPDIVIDE;
        case '%':
            return FILE_OPMODULO;
        default:
            return -1;
    }
}


#ifdef ENABLE_CONDITIONALS
static int
get_cond(const char* l, const char** t)
{
    static const struct cond_tbl_s
    {
        char name[8];
        size_t len;
        int cond;
    } cond_tbl[] =
    {
        {"if",      2,  COND_IF},
        {"elif",    4,  COND_ELIF},
        {"else",    4,  COND_ELSE},
        {"",        0,  COND_NONE},
    };

    const struct cond_tbl_s* p;
    for(p = cond_tbl; p->len; p++)
    {
        if(strncmp(l, p->name, p->len) == 0 &&
            isspace((unsigned char)l[p->len]))
        {
            if(t)
                *t = l + p->len;
            break;
        }
    }
    return p->cond;
}


static int check_cond(struct magic_set* ms, int cond, uint32_t cont_level)
{
    int last_cond;
    last_cond = ms->c.li[cont_level].last_cond;

    switch(cond)
    {
        case COND_IF:
            if(last_cond != COND_NONE && last_cond != COND_ELIF)
            {
                if(ms->flags & MAGIC_SETS)
                    file_magwarn(ms, "syntax error: `if'");
                return -1;
            }
            last_cond = COND_IF;
            break;
        
        case COND_ELIF:
            if(last_cond != COND_IF && last_cond != COND_ELIF)
            {
                if(ms->flags & MAGIC_CHECK)
                    file_magwarn(ms, "syntax error: `elif'");
                return -1;
            }
            last_cond = COND_ELIF;
            break;

        case COND_ELSE:
            if(last_cond != COND_IF && last_cond != COND_ELIF)
            {
                if(ms->flags & MAGIC_CHECK)
                    file_magwarn(ms, "syntax error: `else'");
                return -1;
            }
            last_cond = COND_NONE;
            break;

        case COND_NONE:
            last_cond = COND_NONE;
            break;
    }

    ms->c.li[cont_level].last_cond = last_cond;
    return 0;
}
#endif


/* parse one line from magic file, put into magic[index++] if valid */
static int
parse(struct magic_set* ms, struct magic_entry* me, const char* line,
            size_t lineno, int action)
{
#ifdef ENABLE_CONDITIONALS
    static uint32_t last_cont_level = 0;
#endif
    size_t i;
    struct magic* m;
    const char* l = line;
    char* t;
    int op;
    uint32_t cont_level;
    int32_t diff;

    cont_level = 0;

    /* parse the offset */
    while(*l == '>')
    {
        ++l;    /* step over */
        cont_level++;
    }
#ifdef ENABLE_CONDITIONALS
    if(cont_level == 0 || cont_level > last_cont_level)
        if(file_check_mem(ms, cont_level) == -1)
            return -1;
    last_cont_level = cont_level;
#endif
    if(cont_level != 0)
    {
        if(me->mp == NULL)
        {
            file_magerror(ms, "No current entry for continuation");
            return -1;
        }
        if(me->cont_count == 0)
        {
            file_magerror(ms, "Continuations present with 0 count");
            return -1;
        }
        m = &me->mp[me->cont_count - 1];
        diff = (int32_t)cont_level - (int32_t)m->cont_level;
        if(diff > 1)
            file_magwarn(ms, "New continuation level %u is more "
                            "than one larger than current level %u", cont_level,
                            m->cont_level);
        if(me->cont_count == me->max_count)
        {
            struct magic* nm;
            size_t cnt = me->max_count + ALLOC_CHUNK;
            if((nm = CAST(struct magic*, realloc(me->mp,
                        sizeof(*nm) * cnt))) == NULL)
            {
                file_oomem(ms, sizeof(*nm) * cnt);
                return -1;
            }
            me->mp = m = nm;
            me->max_count = CAST(uint32_t, cnt);
        }
        m = &me->mp[me->cont_count++];
        (void)memset(m, 0, sizeof(*m));
        m->cont_level = cont_level;
    } else
    {
        static const size_t len = sizeof(*m) * ALLOC_CHUNK;
        if(me->mp != NULL)
            return 1;
        if((m = CAST(struct magic*, malloc(len))) == NULL)
        {
            file_oomem(ms, len);
            return -1;
        }
        me->mp = m;
        me->max_count = ALLOC_CHUNK;
        (void)memset(m, 0, sizeof(*m));
        m->factor_op = FILE_FACTOR_OP_NONE;
        m->cont_level = 0;
        me->cont_count = 1;
    }
    m->lineno = CAST(uint32_t, lineno);

    if(*l == '&')
    {
        /* m->cont_level == 0 checked below */
        ++l;    /* step over */
        m->flag |= OFFADD;
    }
    if(*l == '(')
    {
        ++l;
        m->flag |= INDIR;
        if(m->flag & OFFADD)
            m->flag = (m->flag & ~OFFADD) | INDIROFFADD;

        if(*l == '&')
        {
            /* m->cont_level == 0 checked below */
            ++l;
            m->flag |= OFFADD;
        }
    }

    /* Indirect offsets are not valid at level 0 */
    if(m->cont_level == 0 && (m->flag & (OFFADD | INDIROFFADD)))
        if(ms->flags & MAGIC_CHECK)
            file_magwarn(ms, "relative offset at level 0");

    /* get offset, then skip over it */
    m->offset = (uint32_t)strtoul(l, &t, 0);
    if(l == t)
        if(ms->flags & MAGIC_CHECK)
            file_magwarn(ms, "offset `%s' invalid", l);
    l = t;

    if(m->flag & INDIR)
    {
        m->in_type = FILE_LONG;
        m->in_offset = 0;
        /* read [.lbs][+-]nnnnn */
        if(*l == '.')
        {
            ++l;
            switch(*l)
            {
                case 'l':
                    m->in_type = FILE_LELONG;
                    break;
                case 'L':
                    m->in_type = FILE_BELONG;
                    break;
                case 'm':
                    m->in_type = FILE_MELONG;
                    break;
                case 'h':
                case 's':
                    m->in_type = FILE_LESHORT;
                    break;
                case 'H':
                case 'S':
                    m->in_type = FILE_BESHORT;
                    break;
                case 'c':
                case 'b':
                case 'C':
                case 'B':
                    m->in_type = FILE_BYTE;
                    break;
                case 'e':
                case 'f':
                case 'g':
                    m->in_type = FILE_LEDOUBLE;
                    break;
                case 'E':
                case 'F':
                case 'G':
                    m->in_type = FILE_BEDOUBLE;
                    break;
                case 'i':
                    m->in_type = FILE_LEID3;
                    break;
                case 'I':
                    m->in_type = FILE_BEID3;
                    break;
                default:
                    if(ms->flags & MAGIC_CHECK)
                        file_magwarn(ms,
                                        "indirect offset type `%c' invalid",
                                        *l);
                    break;
            }
            l++;
        }

        m->in_op = 0;
        if(*l == '~')
        {
            m->in_op |= FILE_OPINVERSE;
            l++;
        }
        if((op = get_op(*l)) != -1)
        {
            m->in_op |= op;
            l++;
        }
        if(*l == '(')
        {
            m->in_op |= FILE_OPINDIRECT;
            l++;
        }

        if(isdigit((unsigned char)*l) || *l == '-')
        {
            m->in_offset = (int32_t)strtol(l, &t, 0);
            if(l == t)
                if(ms->flags & MAGIC_CHECK)
                    file_magwarn(ms, "in_offset `%s' invalid", l);
            l = t;
        }
        if(*l++ != ')' ||
             ((m->in_op & FILE_OPINDIRECT) && *l++ != ')'))
            if(ms->flags & MAGIC_CHECK)
                file_magwarn(ms, "missing ')' in indirect offset");
    }
    EATAB;

#ifdef ENABLE_CONDITIONALS
    m->cond = get_cond(l, &l);
    if(check_cond(ms, m->cond, cont_level) == -1)
        return -1;
    EATAB;
#endif

    /* Parse the type */
    if(*l == 'u')
    {
        /* Try it as a keyword type prefixed by "u"; match what
           follows the "u". If that fails, try it as an SUS
           integer type. */
        m->type = get_type(type_tbl, l + 1, &l);
        if(m->type == FILE_INVALID)
        {
            /* Not a keyword type; parse it as an SUS type,
               'u' possibly followed by a number or C/S/L */
            m->type = get_standard_integer_type(l, &l);
        }

        /* It's unsigned */
        if(m->type != FILE_INVALID)
            m->flag |= UNSIGNED;
    } else 
    {
        /* Try it as a keyword type. If that fails, try it as
           an SUS integer type if it begins with "d" or as an
           SUS string type if it begins with "s". In any case,
           it's not unsigned */
        m->type = get_type(type_tbl, l, &l);
        if(m->type == FILE_INVALID)
        {
            /* Not a keyword type; parse it as an SUS type,
               either 'd' possibly followed by a number or
               C/S/L, or just 's' */
            if(*l == 'd')
                m->type = get_standard_integer_type(l, &l);
            else if(*l == 's' && !isalpha((unsigned char)l[1]))
            {
                m->type = FILE_STRING;
                ++l;
            }
        }
    }

    if(m->type == FILE_INVALID)
    {
        /* Not found - try it as a special keyword */
        m->type = get_type(special_tbl, l, &l);
    }

    if(m->type == FILE_INVALID)
    {
        if(ms->flags & MAGIC_CHECK)
            file_magwarn(ms, "type `%s' invalid", l);
        return -1;
    }

    /* New-style anding: "0 byte&0x80 =0x80 dynamically linked */

    m->mask_op = 0;
    if(*l == '~')
    {
        if(!IS_STRING(m->type))
            m->mask_op |= FILE_OPINVERSE;
        else if(ms->flags & MAGIC_CHECK)
            file_magwarn(ms, "'~' invalid for string type");
        ++l;
    }
    m->str_range = 0;
    m->str_flags = m->type == FILE_PSTRING ? PSTRING_1_LE : 0;
    if((op = get_op(*l)) != -1)
    {
        if(!IS_STRING(m->type))
        {
            uint64_t val;
            ++l;
            m->mask_op |= op;
            val = (uint64_t)strtoull(l, &t, 0);
            l = t;
            m->num_mask = file_signextend(ms, m, val);
            eatsize(&l);
        }
        else if(op == FILE_OPDIVIDE)
        {
            int have_range = 0;
            while(!isspace((unsigned char)*++l))
            {
                switch(*l)
                {
                    case '0': case '1': case '2':
                    case '3': case '4': case '5':
                    case '6': case '7': case '8':
                    case '9':
                        if(have_range &&
                            (ms->flags & MAGIC_CHECK))
                            file_magwarn(ms, "multiple ranges");
                        have_range = 1;
                        m->str_range = CAST(uint32_t,
                                        strtoul(l, &t, 0));
                        if(m->str_range == 0)
                            file_magwarn(ms, "zero range");
                        l = t - 1;
                        break;
                    case CHAR_COMPACT_WHITESPACE:
                        m->str_flags |= STRING_COMPACT_WHITESPACE;
                        break;
                    case CHAR_COMPACT_OPTIONAL_WHITESPACE:
                        m->str_flags |= STRING_COMPACT_OPTIONAL_WHITESPACE;
                        break;
                    case CHAR_IGNORE_LOWERCASE:
                        m->str_flags |= STRING_IGNORE_LOWERCASE;
                        break;
                    case CHAR_IGNORE_UPPERCASE:
                        m->str_flags |= STRING_IGNORE_UPPERCASE;
                        break;
                    case CHAR_REGEX_OFFSET_START:
                        m->str_flags |= REGEX_OFFSET_START;
                        break;
                    case CHAR_BINTEST:
                        m->str_flags |= STRING_BINTEST;
                        break;
                    case CHAR_TEXTTEST:
                        m->str_flags |= STRING_TEXTTEST;
                        break;
                    case CHAR_TRIM:
                        m->str_flags |= STRING_TRIM;
                        break;
                    case CHAR_PSTRING_1_LE:
                        if(m->type != FILE_PSTRING)
                            goto bad;
                        m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_1_LE;
                        break;
                    case CHAR_PSTRING_2_BE:
                        if(m->type != FILE_PSTRING)
                            goto bad;
                        m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_2_BE;
                        break;
                    case CHAR_PSTRING_2_LE:
                        if(m->type != FILE_PSTRING)
                            goto bad;
                        m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_2_LE;
                        break;
                    case CHAR_PSTRING_4_BE:
                        if(m->type != FILE_PSTRING)
                            goto bad;
                        m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_4_BE;
                        break;
                    case CHAR_PSTRING_4_LE:
                        if(m->type != FILE_PSTRING)
                            goto bad;
                        m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_4_LE;
                        break;
                    case CHAR_PSTRING_LENGTH_INCLUDES_ITSELF:
                        if(m->type != FILE_PSTRING)
                            goto bad;
                        m->str_flags |= PSTRING_LENGTH_INCLUDES_ITSELF;
                        break;
                    default:
                    bad:
                        if(ms->flags & MAGIC_CHECK)
                            file_magwarn(ms,
                                    "string extension `%c' "
                                    "invalid", *l);
                        return -1;
                }

                /* Allow multiple '/' for readability */
                if(l[1] == '/' &&
                    !isspace((unsigned char)l[2]))
                    l++;
            }
            if(string_modifier_check(ms, m) == -1)
                return -1;
        } else
        {
            if(ms->flags & MAGIC_CHECK)
                file_magwarn(ms, "invalid string op: %c", *t);
            return -1;
        }
    }

    /* We used to set mask to all 1's here, instead let's just not do
       anything if mask = 0 (unless you have a better idea) */
    EATAB;

    switch(*l)
    {
        case '>':
        case '<':
            m->reln = *l;
            ++l;
            if(*l == '=')
            {
                if(ms->flags & MAGIC_CHECK)
                {
                    file_magwarn(ms, "%c= not supported", m->reln);
                    return -1;
                }
                ++l;
            }
            break;
        /* Old-style anding: "0 byte &0x80 dynamically linked */
        case '&':
        case '^':
        case '=':
            m->reln = *l;
            ++l;
            if(*l == '=')
            {
                /* HP compat: ignore &= etc */
                ++l;
            }
            break;
        case '!':
            m->reln = *l;
            ++l;
            break;
        default:
            m->reln = '='; /* the default relation */
            if(*l == 'x' && ((isascii(unsigned char)l[1]) &&
                isspace((unsigned char)l[1]) || !l[1]))
            {
                m->reln = *l;
                ++l;
            }
            break;
    }

    /* Grab the value part, except for an 'x' reln */
    if(m->reln != 'x' && getvalue(ms, m, &l, action))
        return -1;

    /* Now get last part - the description */
    EATAB;

    if(l[0] == '\b')
    {
        ++l;
        m->flags |= NOSPACE;
    } else if((l[0] == '\\') && (l[1] == 'b'))
    {
        ++l;
        ++l;
        m->flag |= NOSPACE;
    }
    for(i = 0; (m->desc[i++] = *l++) != '\0' && i < sizeof(m->desc); )
        continue;
    if(i == sizeof(m->desc))
    {
        m->desc[sizeof(m->desc) - 1] = '\0';
        if(ms->flags & MAGIC_CHECK)
            file_magwarn(ms, "description `%s' truncated", m->desc);
    }

    /* We only do this check while compiling, or if any of the magic
       files were not compiled */
    if(ms->flags & MAGIC_CHECK)
    {
        if(check_format(ms, m) == -1)
            return -1;
    }

    if(action == FILE_CHECK)
        file_mdump(m);

    m->mimetype[0] = '\0';      /* initialize MIME type to none */
    return 0;
}




/* Load and parse one file */
static void
load_1(struct magic_set* ms, int action, const char* fn, int *errs,
        struct magic_entry** mentry, uint32_t *mentrycount)
{
    size_t lineno = 0, llen = 0;
    char* line = NULL;
    ssize_t len;
    struct magic_entry me;

    FILE* f = fopen(ms->file = fn, "r");
    if(f == NULL)
    {
        if(errno != ENOENT)
            file_error(ms, errno, "cannot read magic file `%s'",
                        fn);
            (*errs)++;
            return;
    }

    memset(&me, 0, sizeof(me));
    /* read and parse this file */
    for(ms->line = 1; (len = getline(&line, &llen, f)) != -1;
            ms->line++)
    {
        if(len == 0)    /* null line, garbage, etc */
            continue;
        if(line[len - 1] == '\n')
        {
            lineno++;
            line[len - 1] = '\0';   /* delete newline */
        }
        switch(line[0])
        {
            case '\0':      /* empty, do not parse */
            case '#':       /* comment, do not parse */
                continue;
            case '!':
                if(line[1] == ':')
                {
                    size_t i;
                    for(i = 0; bang[i].name != NULL; i++)
                    {
                        if((size_t)(len - 2) > bang[i].len &&
                            memcmp(bang[i].name, line + 2,
                            bang[i].len) == 0)
                            break;
                    }
                    if(bang[i].name == NULL)
                    {
                        file_error(ms, 0,
                                     "Unknown !: entry `%s'", line);
                        (*errs)++;
                        continue;
                    }
                    if(me.mp == NULL)
                    {
                        file_error(ms, 0,
                                    "No current entry for :!%s type",
                                    bang[i].name);
                        (*errs)++;
                        continue;
                    }
                    if((*bang[i].fun)(ms, &me,
                                    line + bang[i].len + 2) != 0)
                    {
                        (*errs)++;
                        continue;
                    }
                    continue;
                }
            default:
            again:
                switch(parse(ms, &me, line, lineno, action))
                {
                    case 0:
                        continue;
                    case 1:
                        (void)addentry(ms, &me, mentry, mentrycount);
                        goto again;
                    default:
                        (*errs)++;
                        break;
                }
        }
    }
    if(me.mp)
        (void)addentry(ms, &me, mentry, mentrycount);
    free(line);
    (void)fclose(f);
}


static struct magic_map*
apprentice_load(struct magic_set* ms, const char* fn, int action)
{
    int errs = 0;
    struct magic_entry* mentry[MAGIC_SETS] = { NULL };
    uint32_t mentrycount[MAGIC_SETS] = { 0 };
    uint32_t i, j;
    size_t files = 0, maxfiles = 0;
    char** filearr = NULL;
    char* mfn;
    struct stat st;
    struct magic_map* map;
    DIR* dir;
    struct dirent* d;

    ms->flags |= MAGIC_CHECK;       /* Enable checks for parsed files */

    if((map = CAST(struct magic_map*, calloc(1, sizeof(*map)))) == NULL)
    {
        file_oomem(ms, sizeof(*map));
        return NULL;
    }

    /* print silly verbose header for USG compat */
    if(action == FILE_CHECK)
        (void)fprintf(stderr, "%s\n", usg_hdr);

    /* load directory or file */
    if(stat(fn, &st) == 0 && S_ISDIR(st.st_mode))
    {
        dir = opendir(fn);
        if(!dir)
        {
            errs++;
            goto out;
        }
        while((d = readdir(dir)) != NULL)
        {
            if(asprintf(&mfn, "%s/%s", fn, d->d_name) < 0)
            {
                file_oomem(ms,
                      strlen(fn) + strlen(d->d_name) + 2);
                errs++;
                closedir(dir);
                goto out;
            }
            if(stat(mfn, &st) == -1 || !S_ISREG(st.st_mode))
            {
                free(mfn);
                continue;
            }
            if(files >= maxfiles)
            {
                size_t mlen;
                maxfiles = (maxfiles + 1) * 2;
                mlen = maxfiles * sizeof(*filearr);
                if((filearr = CAST(char**,
                                realloc(filearr, mlen))) == NULL)
                {
                    file_oomem(ms, mlen);
                    free(mfn);
                    closedir(dir);
                    errs++;
                    goto out;
                }
            }
            filearr[files++] = mfn;
        }
        closedir(dir);
        qsort(filearr, files, sizeof(*filearr), cmpstrp);
        for(i = 0; i < files; i++)
        {
            load_1(ms, action, filearr[i], &errs, mentry,
                    mentrycount);
            free(filearr[i]);
        }
        free(filearr);
    } else
        load_1(ms, action, fn, &errs, mentry, mentrycount);
    if(errs)
        goto out;

    for(j = 0; j < MAGIC_SETS; j++)
    {
        /* Set types of tests */
        for(i = 0; i < mentrycount[j]; )
        {
            if(mentry[j][i].mp->cont_level != 0)
            {
                i++;
                continue;
            }
            i = set_text_binary(ms, mentry[j], mentrycount[j], i);
        }
        qsort(mentry[j], mentrycount[j], sizeof(*mentry[j]),
                apprentice_sort);

        /* Make sure that any level 0 "default" line is last
           (if one exists) */
        set_last_default(ms, mentry[j], mentrycount[j]);

        /* coalesce per file arrays into a single one */
        if(coalesce_entries(ms, mentry[j], mentrycount[j],
                &map->magic[j], &map->nmagic[j]) == -1)
        {
            errs++;
            goto out;
        }
    }
out:
    for(j = 0; j < MAGIC_SETS; j++)
        magic_entry_free(mentry[j], mentrycount[j]);

    if(errs)
    {
        for(j = 0; j < MAGIC_SETS; j++)
        {
            if(map->magic[j])
                free(map->magic[j]);
        }
        free(map);
        return NULL;
    }
    return map;
}



static void
apprentice_unmap(struct magic_map* map)
{
    if(map == NULL)
        return;
    if(map->p == NULL)
        return;

    if(map->len)
        (void)munmap(map->p, map->len);
    else
        free(map->p);

    free(map);
}

static void
mlist_free(struct mlist* mlist)
{
    struct mlist* ml;

    if(mlist == NULL)
        return;

    for(ml = mlist->next; ml != mlist;)
    {
        struct mlist* next = ml->next;
        if(ml->map)
            apprentice_unmap(ml->map);
        free(ml);
        ml = next;
    }
    free(ml);
}


/* Handle one file or directory */
static int
apprentice_1(struct magic_set* ms, const char* fn, int action)
{
    struct mlist* ml;
    struct magic_map* map;
    size_t i;

    if(magicsize != FILE_MAGICSIZE)
    {
        file_error(ms, 0, "magic element size %lu != %lu",
                (unsigned long)sizeof(*map->magic[0]),
                (unsigned long)FILE_MAGICSIZE);
        return -1;
    }

    if(action == FILE_COMPILE)
    {
        map = apprentice_load(ms, fn, action);
        if(map == NULL)
            return -1;
        return apprentice_compile(ms, map, fn);
    }

    map = apprentice_map(ms, fn);
    if(map == NULL)
    {
        if(ms->flags & MAGIC_CHECK)
            file_magwarn(ms, "using regular magic file `%s'", fn);
        map = apprentice_load(ms, fn, action);
        if(map == NULL)
            return -1;
    }

    for(i = 0; i < MAGIC_SETS; i++)
    {
        if(add_mlist(ms->mlist[i], map, i) == -1)
        {
            file_oomem(ms, sizeof(*ml));
            apprentice_unmap(map);
            return -1;
        }
    }

    if(action == FILE_LIST)
    {
        for(i = 0; i < MAGIC_SETS; i++)
        {
            printf("Set %zu:\nBinary patterns:\n", i);
            apprentice_list(ms->mlist[i], BINTEST);
            printf("Text patterns:\n");
            apprentice_list(ms->mlist[i], TEXTTEST);
        }
    }

    return 0;
}

static struct mlist*
mlist_alloc(void)
{
    struct mlist* mlist;
    if((mlist = CAST(struct mlist*, calloc(1, sizeof(*mlist)))) == NULL)
    {
        return NULL;
    }
    mlist->next = mlist->prev = mlist;
    return mlist;
}

/* const char* fn: list of magic files and directories */
int file_apprentice(struct magic_set* ms, const char* fn, int action)
{
    char *p, *mfn;
    int file_err, errs = -1;
    size_t i;

    if((fn = magic_getpath(fn, action)) == NULL)
        return -1;

    init_file_tables();

    if((mfn = strdup(fn)) == NULL)
    {
        file_oomem(ms, strlen(fn));
        return -1;
    }

    for(i = 0; i < MAGIC_SETS; i++)
    {
        mlist_free(ms->mlist[i]);
        if((ms->mlist[i] = mlist_alloc()) == NULL)
        {
            file_oomem(ms, sizeof(*ms->mlist[i]));
            if(i != 0)
            {
                --i;
                do
                    mlist_free(ms->mlist[i]);
                while (i != 0);
            }
            free(mfn);
            return -1;
        }
    }

    fn = mfn;

    while(fn)
    {
        p = strchr(fn, PATHSEP);
        if(p)
            *p++ = '\0';
        if(*fn == '\0')
            break;
        file_err = apprentice_l(ms, fn, action);
        errs = MAX(errs, file_err);
        fn = p;
    }

    free(mfn);

    if(errs == -1)
    {
        for(i = 0; i < MAGIC_SETS; i++)
        {
            mlist_free(ms->mlist[i]);
            ms->mlist[i] = NULL;
        }
        file_error(ms, 0, "could not find any valid magic files!");
        return -1;
    }

    if(action == FILE_LOAD)
        return 0;

    for(i = 0; i < MAGIC_SETS; i++)
    {
        mlist_free(ms->mlist[i]);
        ms->mlist[i] = NULL;
    }

    switch(action)
    {
        case FILE_COMPILE:
        case FILE_CHECK:
        case FILE_LIST:
            return 0;
        default:
            file_error(ms, 0, "Invalid action %d", action);
            return -1;
    }
}
