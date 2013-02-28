/* operand2sig.c -- common function for parsing signal specifications */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <error.h>
#include <errno.h>
#include <string.h>

#include "operand2sig.h"
#include "xalloc.h"
#include "sig2str.h"
#include "gettext.h"

#define _(msgid) gettext(msgid)

#ifndef ISDIGIT
# define ISDIGIT(c) ((unsigned int)(c) - '0' <= 9)
#endif

int operand2sig(char* operand, char* signame)
{
    int signum;

    if(ISDIGIT(*operand))
    {
        char* endp;
        long int l = (errno = 0, strtol(operand, &endp, 10));
        long i = l;
        signum = (operand == endp || *endp || errno 
                  || i != l
                     ? -1 : WIFSIGNALED(i) ? WTERMSIG(i) : i);
    }
    else
    {
        /* Convert signal to upper case in the C locale, not in the
           current locale. Don't assume ASCII; it might be EBCDIC */
        char* upcased = xstrdup(operand);
        char* p;
        for(p = upcased; *p; p++)
            if(strchr("abcdefghijklmnopqrstuvwxyz", *p))
                *p += 'A' - 'a';

        /* Look for the signal name, possibly prefixed by "SIG",
           and possibly lowercased. */
        if(!(str2sig(upcased, &signum) == 0
            || (upcased[0] == 'S' && upcased[1] == 'I' && upcased[2] == 'G'
                && str2sig(upcased + 3, &signum) == 0)))
            signum = -1;

        free(upcased);
    }

    if(signum < 0 || sig2str(signum, signame) != 0)
    {
        error(0, 0, _("%s: invalid signal"), operand);
        return -1;
    }

    return signum;
}

