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

/* Read an arbitrarily long line of text from STREAM into LINEBUFFER.
   Consider lines to be terminated by DELIMITER.
   Keep the delimiter; append DELIMITER if it's the last line of a file
   that ends in a character other than DELIMITER. Do not NUL-terminate.
   Therefore the stream can contain NUL bytes, and the length
   (including the delimiter) is returned in linebuffer->length.
   Return NULL when stream is empty. Return NULL and set errno upon
   error; callers can distinguish this case from the empty case by
   invoking ferror(stream).
   Otherwise, return LINEBUFFER */
struct linebuffer *
readlinebuffer_delim(struct linebuffer *linebuffer, FILE *stream,
                    char delimiter)
{
    int c;
    char *buffer = linebuffer->buffer;
    char *p = linebuffer->buffer;
    char *end = buffer + linebuffer->length;    /* sentinel */

    if(feof(stream))
        return NULL;

    do
    {
        c = getc(stream);
        if(c == EOF)
        {
            if(p == buffer || ferror(stream))
                return NULL;
            if(p[-1] == delimiter)
                break;
            c = delimiter;
        }
        if(p == end)
        {
            size_t oldsize = linebuffer->size;
            buffer = x2realloc(buffer, &linebuffer->size);
            p = buffer + oldsize;
            linebuffer->buffer = buffer;
            end = buffer + linebuffer->size;
        }
        *p++ = c;
    } while(c != delimiter);

    linebuffer->length = p - buffer;
    return linebuffer;
}


struct linebuffer*
readlinebuffer(struct linebuffer *linebuffer, FILE *stream)
{
    return readlinebuffer_delim(linebuffer, stream, '\n');
}
