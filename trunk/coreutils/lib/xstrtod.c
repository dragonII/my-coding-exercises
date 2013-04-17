/* Error-checking interface to strtod-like functions */

#include "xstrtod.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>

//#define XSTRTOD xstrtod
//#define XSTRTOD xstrtold
#define xstrtod XSTRTOD
#define xstrtold XSTRTOD
#define DOUBLE double

/* An interface to a string-to-floating-pointe conversion function that
   encapsulates all the error checking one should usually perform.
   Like strtod/strtold, but upon successful
   conversion put the result in *RESULT and return true. Return
   false and don't modify *RESULT upon any failure. CONVERT
   specifies the conversion function, e.g., strtod itself. */
bool XSTRTOD(char* str, const char** ptr, DOUBLE* result,
                DOUBLE (*convert)(char*, char**))
{
    DOUBLE val;
    char* terminator;
    bool ok = true;

    errno = 0;
    val = convert(str, &terminator);

    /* Having a non-zero terminator is an error only when PTR is NULL */
    if(terminator == str || (ptr == NULL && *terminator != '\0'))
        ok = false;
    else
    {
        /* Allow underflow (in which case CONVERT returns zero),
           but flag overflow as an error */
        if(val != 0 && errno == ERANGE)
            ok = false;
    }

    if(ptr != NULL)
        *ptr = terminator;

    *result = val;
    return ok;
}

