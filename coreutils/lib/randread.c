/* Generate buffers of random data */

#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <string.h>

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

#ifndef ALIGNED_POINTER
# define alignof(type) offsetof(struct { char c; type x; }, x)
# define ALIGNED_POINTER(ptr, type) ((size_t)(ptr) % alignof(type) == 0)
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


/* Place SIZE random bytes into the buffer beginning at P, using
   the stream in S. */
static void
readsource(struct randread_source* s, unsigned char* p, size_t size)
{
    for(;;)
    {
        size_t inbytes = fread(p, sizeof *p, size, s->source);
        int fread_errno = errno;
        p += inbytes;
        size -= inbytes;
        if(size == 0)
            break;
        errno = (ferror(s->source) ? fread_errno : 0);
        s->handler(s->handler_arg);
    }
}


/* Place SIZE pseudorandom bytes into the buffer beginning at P, using
   the buffered ISAAC generator in ISAAC */
static void
readisaac(struct isaac* isaac, unsigned char* p, size_t size)
{
    size_t inbytes = isaac->buffered;

    for(;;)
    {
        if(size <= inbytes)
        {
            memcpy(p, isaac->data.b + ISAAC_BYTES - inbytes, size);
            isaac->buffered = inbytes - size;
            return;
        }

        memcpy(p, isaac->data.b + ISAAC_BYTES - inbytes, inbytes);
        p += inbytes;
        size -= inbytes;

        /* If P is aligned, write to *P directly to avoid the overhead
           of copying from the buffer */
        if(ALIGNED_POINTER(p, uint32_t))
        {
            uint32_t* wp = (uint32_t*)p;
            while(ISAAC_BYTES <= size)
            {
                isaac_refill(&isaac->state, wp);
                wp += ISAAC_WORDS;
                size -= ISAAC_BYTES;
                if(size == 0)
                {
                    isaac->buffered = 0;
                    return;
                }
            }
            p = (unsigned char *)wp;
        }
        isaac_refill(&isaac->state, isaac->data.w);
        inbytes = ISAAC_BYTES;
    }
}


/* Consume random data from *S to generate a random buffer BUF of size
   SIZE */
void randread(struct randread_source* s, void* buf, size_t size)
{
    if(s->source)
        readsource(s, buf, size);
    else
        readisaac(&s->buf.isaac, buf, size);
}


/* Clear *S so that it no longer contains undelivered random data, and
   deallocate any system resources associated with *S. Return 0 if
   successful, a negative number (setting errno) if not (this is rare,
   but can occur in theory if there is an input error) */
int randread_free(struct randread_source* s)
{
    FILE* source = s->source;
    memset(s, 0, sizeof *s);
    free(s);
    return(source ? fclose(source) : 0);
}
