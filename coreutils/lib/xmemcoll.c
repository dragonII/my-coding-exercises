/* Locale-specific memory comparison */

#include <errno.h>
#include <error.h>
#include <stdlib.h>

#include "gettext.h"
#ifndef _
# define _(msgid) gettext(msgid)
#endif

#include "exitfail.h"
#include "xmemcoll.h"
#include "memcoll.h"

/* Compare S1 (with length S1LEN) and S2 (with length S2LEN) according
   to the LC_COLLATE locale. S1 and S2 do not overlap, and are not
   adjacent. Temporarily modify the bytes after S1 and S2, but
   restore their original contents before returning. Report an error
   and exit if there is an error */

int xmemcoll(char *s1, size_t s1len, char *s2, size_t s2len)
{
    int diff = memcoll(s1, s1len, s2, s2len);
    int collation_errno = errno;

    if(collation_errno)
    {
        error(0, collation_errno, _("string comparison failed"));
        error(0, 0, _("Set LC_ALL='C' to work around the problem."));
        error(exit_failure, 0,
            _("The strings compared were %s and %s."),
            s1, s2);
    }

    return diff;
}
