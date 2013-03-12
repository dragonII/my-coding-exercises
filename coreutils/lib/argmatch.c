/* argmatch.c -- find a match for a string in an array */

#include "argmatch.h"
#include "gettext.h"

#include <sys/types.h>
#include <error.h>
#include <string.h>

#ifndef _(msgid)
#define _(msgid) gettext(msgid)
#endif

/* Error reporting for argmatch.
   CONTEXT is a description of the type of entity that was being matched.
   VALUE is the invalid value that was given.
   PROBLEM is the return value from argmatch */
void
argmatch_invalid(char* context, char* value, ptrdiff_t problem)
{
    char* format = (problem == -1
                    ? _("invalid argument %s for %s")
                    : _("ambiguous argument %s for %s"));

    error(0, 0, format, quote(value), quote(context));
}

/* List the valid arguments for argmatch.
   ARGLIST is the same as in argmatch.
   VALLIST is a pointer to an array of values.
   VALSIZE is the size of the elements of VALLIST. */
void argmatch_valid(char** arglist,
                    char* vallist, size_t valsize)
{
    size_t i;
    char* last_val = NULL;

    /* We try to put synonyms on the same line. The assumption is that
       synonyms follow each other */
    fprintf(stderr, _("Valid arguments are:"));
    for(i = 0; arglist[i]; i++)
        if((i == 0)
            || memcmp(last_val, vallist + valsize * i, valsize))
        {
            fprintf(stderr, "\n  -  `%s'", arglist[i]);
            last_val = vallist + valsize * i;
        }
        else
            fprintf(stderr, ",  `%s'", arglist[i]);

        putc('\n', stderr);
}

/* Never failing versions of the previous functions.

   CONTEXT is the context for which argmatch is called (e.g.,
   "--version-control", or "$VERSION_CONTROL" etc). Upon failure,
   calls the (supposed never to return) function EXIT_FN */
ptrdiff_t
__xargmatch_internal(char* context,
                     char* arg, char** arglist,
                     char* vallist, size_t valsize,
                     argmatch_exit_fn exit_fn)
{
    ptrdiff_t res = argmatch(arg, arglist, vallist, valsize);
    if(res >= 0)
        /* Success */
        return res;

    /* We failed. Explain why */
    argmatch_invalid(context, arg, res);
    argmatch_valid(arglist, vallist, valsize);
    (*exit_fn)();

    return -1;
}
