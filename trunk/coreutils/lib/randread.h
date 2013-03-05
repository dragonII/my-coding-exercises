/* Generate buffers of random data */

#ifndef RANDREAD_H
#define RANDREAD_H

#include <stdio.h>

#include "rand-isaac.h"


/* The maximum buffer size used for reads of random data. Using the
   value 2 * ISAAC_BYTES makes this the largest power of two that
   would not otherwise cause struct randread_source to grow */
#define RANDREAD_BUFFER_SIZE (2 * ISAAC_BYTES)

/* A source of random data for generating random buffers */
struct randread_source
{
    /* Stream to read random bytes from. If null, the current
       implementation uses an internal PRNG (ISAAC). */
    FILE* source;

    /* Function to call, and its argument, if there is an input error or
       end of file when reading from the stream; errno is nonzero if
       there was an error. If this function returns, it should fix the
       problem before returning. The default handler assumes that
       handler_arg is the file name of the source */
    void (*handler)(void*);
    void *handler_arg;

    /* The buffer for SOURCE. It's kept here to simplify storage
       allocation and to make it easier to clear out buffered random
       data */
    union
    {
        /* The stream buffer, if SOURCE is no null */
        char c[RANDREAD_BUFFER_SIZE];

        /* The buffered ISAAC pseudorandom buffer, if SOURCE is null */
        struct isaac
        {
            /* The number of bytes that are buffered at the end of data.b */
            size_t buffered;

            /* State of the ISAAC generator */
            struct isaac_state state;

            /* Up to a buffer's worth of pseudorandom data */
            union
            {
                uint32_t w[ISAAC_WORDS];
                unsigned char b[ISAAC_BYTES];
            } data;
        } isaac;
    } buf;
};

struct randread_source*
randread_new(char* name, size_t bytes_bound);
void randread(struct randread_source* s, void* buf, size_t size);
int randread_free(struct randread_source* s);

#endif
