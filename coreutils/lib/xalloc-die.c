/* Report a memory allocation failure and exit. */

#include <stdlib.h>
#include <error.h>

#include "exitfail.h"
#include "gettext.h"
#define _(msgid) gettext(msgid)

void xalloc_die(void)
{
    error(exit_failure, 0, "%s", _("memory exhausted"));

    /* The `noreturn' cannot be given to error, since it may return if
       its first argument is 0. To help compilers understand the
       xalloc_die does not return, call abort. Also, the abort is a
       safety feature is exit_failure is 0 (which shouldn't happen). */

   abort();
}
