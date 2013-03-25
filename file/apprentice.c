#include "file.h"
#include "magic_.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


int file_formats[FILE_NAMES_SIZE];
const char* file_names[FILE_NAMES_SIZE];

struct type_tbl_s
{
    const char name[16];
    const size_t len;
    const int  type;
    const int  format;
};


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
