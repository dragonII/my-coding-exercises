/* memory allocation routines with error checking. */

#include "libiberty.h"

#include <unistd.h>

void xmalloc_set_program_name(const char *s)
{
    name = s;
    if(first_break == NULL)
        first_break = (char *)sbrk(0);
}

void xmalloc_failed(size_t size)
{
    extern char **environ;
    size_t allocated;

    if(first_break != NULL)
        allocated = (char *)sbrk(0) - first_break;
    else
        allocated = (char *)sbrk(0) - (char *)&environ;
    fprintf(stderr,
        "\n%s%sout of memory allocating %lu bytes after a total of %lu bytes\n",
        name, *name ? ": " : "",
        (unsigned long)size, (unsigned long)allocated);

    xexit(1);
}

PTR xmalloc(size_t size)
{
    PTR newmem;

    if(size == 0)
        size = 1;
    newmem = malloc(size);
    if(!newmem)
        xmalloc_failed(size);

    return newmem;
}
