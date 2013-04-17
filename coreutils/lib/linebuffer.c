/* linebuffer.c -- read arbitrary long lines */

#include "linebuffer.h"
#include "xalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "memcoll.h"

/* Initialize linebuffer LINEBUFFER for use */
void initbuffer(struct linebuffer *linebuffer)
{
    memset(linebuffer, 0, sizeof *linebuffer);
}


struct linebuffer*
readlinebuffer(struct linebuffer *linebuffer, FILE *stream)
{
    return readlinebuffer_delim(linebuffer, stream, '\n');
}
