/* Convert string to double, using the C locale */

#include "c-strtod.h"

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

//#define C_STRTOD c_strtod
//#define C_STRTOD c_strtold
#define c_strtod C_STRTOD
#define c_strtold C_STRTOD
#define DOUBLE double
#define STRTOD_L strtod_l
#define STRTOD strtod

DOUBLE C_STRTOD(char* nptr, char** endptr)
{
    DOUBLE r;

    char* saved_locale = setlocale(LC_NUMERIC, NULL);
    if(saved_locale)
    {
        saved_locale = strdup(saved_locale);
        if(saved_locale == NULL)
        {
            if(endptr)
                *endptr = (char*)nptr;
            return 0;  /* errno is set here */
        }
        setlocale(LC_NUMERIC, "C");
    }

    r = STRTOD(nptr, endptr);

    if(saved_locale)
    {
        int saved_errno = errno;

        setlocale(LC_NUMERIC, saved_locale);
        free(saved_locale);
        errno = saved_errno;
    }

    return r;
}
