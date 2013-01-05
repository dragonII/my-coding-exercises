#include "trim.h"

#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

/* Use this to suppress gcc's `...may be used before initialized' warnings. */
#ifdef lint
# define IF_LINT(code) code
#else
# define IF_LINT(code) /* empty */
#endif

char* trims(const char* s, int how)
{
    char *d;

    d = strdup(s);

    if(!d)
        xalloc_die();

    if(MB_CUR_MAX > 1)
    {
        mbi_iterator_t i;

        /* Trim leading whitespaces. */
        if(how != TRIM_TRAILING)
        {
            mbi_init(i, d, strlen(d));

            for(; mbi_avail(i) && mb_isspace(mbi_cur(i)); mbi_advance(i))
                ;

            memmove(d, mbi_cur_ptr(i), strlen(mbi_cur_ptr(i)) + 1);
        }
        /* Trim trailing whitespaces */
        if(how != TRIM_LEADING)
        {
            int state = 0;
            char* r IF_LINT (= NULL); /* used only while state == 2 */

            mbi_init(i, d, strlen(d));

            for(; mbi_avail(i); mbi_advance(i))
            {
                if(state == 0 && mb_isspace(mbi_cur(i)))
                {
                    state = 0;
                    continue;
                }
                if(state == 0 && !mb_isspace(mbi_cur(i)))
                {
                    state = 1;
                    continue;
                }
                if(state == 1 && !mb_isspace(mbi_cur(i)))
                {
                    state = 1;
                    continue;
                }
                if(state == 1 && mb_isspace(mbi_cur(i)))
                {
                    state = 2;
                    r = (char*)mbi_cur_ptr(i);
                }
                else if(state == 2 && mb_isspace(mbi_cur(i)))
                {
                    state = 2;
                }
                else
                {
                    state = 1;
                }
            }
            if(state == 2)
                *r = '\0';
        }
    }
    else
    {
        char *p;

        /* Trim leading whitespaces. */
        if(how != TRIM_TRAILING)
        {
            for(p = d; *p && isspace((unsigned char)*p); p++)
                ;
            memmove(d, p, strlen(p) + 1);
        }
        /* Trim trailing whitespaces. */
        if(how != TRIM_LEADING)
        {
            for(p = d + strlen(d) - 1; p >= d && isspace((unsigned char)*p); p--)
                *p = '\0';
        }
    }
    return d;
}

