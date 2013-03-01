/* Generate buffers of random data */

#include <errno.h>
#include <error.h>
#include <stdbool.h>

#include "randread.h"
#include "xalloc.h"
#include "exitfail.h"
#include "quote.h"
#include "gettext.h"

#include "stdio-safer.h"
#include "unistd-safer.h"

#ifndef ATTRIBUTE_NORETURN
# define ATTRIBUTE_NORETURN __attribute__ ((__noreturn__))
#endif

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef _
# define _(msgid) gettext(msgid)
#endif


/* The default error handler */
static void ATTRIBUTE_NORETURN
randread_error(void* file_name)
{
    if(file_name)
        error(exit_failure, errno,
              _(errno == 0 ? "%s: end of file" : "%s: read error"),
              quote(file_name));
    abort();
}


static struct randread_source*
simple_new(FILE* source, void* handler_arg)
{
    struct randread_source* s = xmalloc(sizeof *s);
    s->source = source;
    s->handler = randread_error;
    s->handler_arg = handler_arg;
    return s;
}

/* Create and initialize a random data source from NAME, or use a
   reasonable default source if NAME is null. BYTES_BOUND is an upper
   bound on the number of bytes that will needed. If zero, it is a
   hard bound; otherwise it is just an estimate.

   If NAME is not null, NAME is saved for use as the argument of the
   default handler. Unless a non-default handler is used, NAME's
   lifetime should be at least that of the returned value.

   Return NULL (setting errno) on failure */
struct randread_source*
randread_new(char* name, size_t bytes_bound)
{
    if(bytes_bound == 0)
        return simple_new(NULL, NULL);
    else
    {
        FILE* source = NULL;
        struct randread_source* s;

        if(name)
            if(!(source = fopen_safer(name, "rb")))
                return NULL;

        s = simple_new(source, name);

        if(source)
            setvbuf(source, s->buf.c, _IOFBF, MIN(sizeof s->buf.c, bytes_bound));
        else
        {
            s->buf.isaac.buffered = 0;
            isaac_seed(&s->buf.isaac.state);
        }

        return s;
    }
}
