/* linebuffer.h -- declarations for reading arbitrary long lines */

#ifndef LINEBUFFER_H
#define LINEBUFFER_H

#include <stdio.h>

/* A `struct linebuffer' holds a line of text */

struct linebuffer
{
    size_t size;            /* allocated */
    size_t length;          /* used */
    char *buffer;
};


void initbuffer(struct linebuffer *);
struct linebuffer*
readlinebuffer(struct linebuffer *, FILE *);

#endif
