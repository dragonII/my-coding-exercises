/* argmatch.h -- definitions and prototypes for argmatch.c */

#ifndef ARGMATCH_H_
#define ARGMATCH_H_

#include <stddef.h>
#include <sys/types.h>

#include "verify.h"

/* xargmatch calls this function when it fails. This function should not
   return. By default, this is a function that calls ARGMATCH_DIE which
   in turn defaults to `exit (exit_failure)'. */
typedef void (*argmatch_exit_fn)(void);
extern argmatch_exit_fn argmatch_die;

ptrdiff_t __xargmatch_internal(char* context,
                               char* arg, char** arglist,
                               char* vallist, size_t valsize,
                               argmatch_exit_fn exit_fn);

/* Report on stderr why argmatch failed. Report correct values */
void argmatch_invalid(char* context, char* value, ptrdiff_t problem);

/* Report on stderr the list of possible arguments */
void argmatch_valid(char** arglist, char* vallist, size_t valsize);


#define ARRAY_CARDINALITY(Array) (sizeof (Array) / sizeof *(Array))


/* Assert there are as many real argments as there are values
   (argument list ends with a NULL guard). */
#define ARGMATCH_VERIFY(Arglist, Vallist)   \
   verify( ARRAY_CARDINALITY (Arglist) == ARRAY_CARDINALITY (Vallist) + 1)


/* Programmer friendly interface to __xargmatch_internal */

#define XARGMATCH(Context, Arg, Arglist, Vallist)               \
    ((Vallist) [__xargmatch_internal (Context, Arg, Arglist,    \
                                        (char*) (Vallist),      \
                                        sizeof *(Vallist),      \
                                        argmatch_die)])


#endif
