/* A more useful interface to strtol. */

#ifndef _XSTRTOL_H
#define _XSTRTOL_H

#include <getopt.h>
#include <inttypes.h>

#ifndef _STRTOL_ERROR
enum strtol_error
{
    LONGINT_OK = 0,

    /* These two values can be ORed together, to indicate that both
       errors occured. */
    LONGINT_OVERFLOW = 1,
    LONGINT_INVALID_SUFFIX_CHAR = 2,

    LONGINT_INVALID_SUFFIX_CHAR_WHIT_OVERFLOW = (LONGINT_INVALID_SUFFIX_CHAR | LONGINT_OVERFLOW),

    LONGINT_INVALID = 4
};

typedef enum strtol_error strtol_error;
#endif  /* strtol_error */

#define _DECLARE_XSTRTOL(name, type) \
   strtol_error name(char*, char**, int, type*, char*);
_DECLARE_XSTRTOL (xstrtol, long int)
_DECLARE_XSTRTOL (xstrtoul, unsigned long int)
_DECLARE_XSTRTOL (xstrtoimax, intmax_t)
_DECLARE_XSTRTOL (xstrtoumax, uintmax_t)

#ifndef __attribute__
# if __GNU__ < 2 || (__GNU__ == 2 && _GNU_MINOR__ < 8)
#   define __attribute__(x)
# endif
#endif

#ifndef ATTRIBUTE_NORETURN
# define ATTRIBUTE_NORETURN __attribute__ ((__noreturn__))
#endif

/* Report an error for an invalid integer in an option argument.
   
   ERR is the error code returned by one of the xstrto* functions.

   Use OPT_IDX to decide whether to print the short option string "C"
   or "-C" or a long option string derived from LONG_OPTION. OPT_IDX
   is -2 if the short option "C" was used, without any leading "-"; it
   is -1 if the short option "-C" was used; otherwise it is an index
   into LONG_OPTIONS, which should have a name preceded by two '-'
   characters.

   ARG is the option-argument containing the ingeger.

   After reporting an error, exit with a failure status. */


#endif
