/* hard-locale.c -- Determine whether a locale is hard */

#include "hard-locale.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GLIBC__
# define GLIBC_VERSION __GLIBC__
#else
# define GLIBC_VERSION 0
#endif

/* Return true if the current CATEGORY locale is hard, i.e. if you
   can't get away with assuming traditional C or POSIX behavior */
bool hard_locale(int category)
{
    bool hard = true;
    char* p = setlocale(category, NULL);

    if(p)
    {
        if(GLIBC_VERSION >= 2)
        {
            if(strcmp(p, "C") == 0 || strcmp(p, "POSIX") == 0)
                hard = false;
        }
        else
        {
            char *locale = strdup(p);
            if(locale)
            {
                /* Temporarily set the locale to the "C" and "POSIX" locales
                   to find their names, so that we can determine whether one
                   or the other is the caller's locale. */
                if(((p = setlocale(category, "C"))
                        && strcmp(p, locale) == 0)
                    || ((p = setlocale(category, "POSIX"))
                        && strcmp(p, locale) == 0))
                    hard = false;

                /* Restore the caller's locale */
                setlocale(category, locale);
                free(locale);
            }
        }
    }

    return hard;
}

