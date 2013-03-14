/* argmatch.c -- find a match for a string in an array */

#include "argmatch.h"
#include "gettext.h"
#include "quote.h"

#include <sys/types.h>
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef _
#define _(msgid) gettext(msgid)
#endif


/* Non failing version of argmatch call this function after failing */
#ifndef ARGMATCH_DIE
# include "exitfail.h"
# define ARGMATCH_DIE exit(exit_failure)
#endif

static void
__argmatch_die(void)
{
    ARGMATCH_DIE;
}

/* Used by XARGMATCH and XARGCASEMATCH. See description in argmatch.h.
   Default to __argmatch_die, but allow caller to change this at run-time. */
argmatch_exit_fn argmatch_die = __argmatch_die;

/* If ARG is an unambiguous match for an element of the
   NULL-terminated array ARGLIST, return the index in ARGLIST
   of the matched element, else -1 if it does not match any element
   or -2 if it is ambiguous (is a prefix of more than one element).

   If VALLIST is none null, use it to resolve ambiguities limited to
   synonyms, i.e., for
     "yes", "yop" -> 0
     "no", "nope" -> 1
   "y" is a valid argument, for `0', and "n" for `1'. */
ptrdiff_t
argmatch(char* arg, char** arglist,
         char* vallist, size_t valsize)
{
    size_t i;                   /* Temporary index in ARGLIST */
    size_t arglen;              /* Length of ARG */
    ptrdiff_t matchind = -1;    /* Index of first nonexact match */
    bool ambiguous = false;     /* If true, multiple nonexact match(es) */

    arglen = strlen(arg);

    /* Test all elements for either exact match or abbreviated matches */
    for(i = 0; arglist[i]; i++)
    {
        if(!strncmp(arglist[i], arg, arglen))
        {
            if(strlen(arglist[i]) == arglen)
                /* Exact match */
                return i;
            else if(matchind == -1)
                /* First nonexact match found */
                matchind = i;
            else
            {
                /* Second nonexact match found */
                if(vallist == NULL
                    || memcmp(vallist + valsize * matchind,
                              vallist + valsize * i, valsize))
                {
                    /* There is a real ambiguity, or we could not
                       disambiguate */
                    ambiguous = true;
                }
            }
        }
    }
    if(ambiguous)
        return -2;
    else
        return matchind;
}

/* Error reporting for argmatch.
   CONTEXT is a description of the type of entity that was being matched.
   VALUE is the invalid value that was given.
   PROBLEM is the return value from argmatch */
void
argmatch_invalid(char* context, char* value, ptrdiff_t problem)
{
    const char* format = (problem == -1
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
