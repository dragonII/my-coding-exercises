/* lib-basename.c -- return the last element in a file name */

#include <string.h>

#include "dirname.h"
#include "xalloc.h"
#include "basename-lgpl.h"
#include "lib-basename.h"
#include "xstrndup.h"

char* base_name(char* name)
{
    char* base = last_component(name);
    size_t length;

    /* If there is no last component, then name is a file system root or the
       empty string. */
    if(! *base)
        return xstrndup(name, base_len(name));

    /* Collapse a sequence of trailing slashes into one. */
    length = base_len(base);
    if(ISSLASH(base[length]))
        length++;

    /* On systems with drive letters, `a/b:c' must return `./b:c' rather
       then `b:c' to avoid confusion with a drive letter. On systems
       with pure POSIX semantics, this is not a issue. */
    if(FILE_SYSTEM_PREFIX_LEN(base))
    {
        char* p = xmalloc(length + 3);
        p[0] = '.';
        p[1] = '/';
        memcpy(p + 2, base, length);
        p[length + 2] = '\0';
        return p;
    }

    /* Finally, copy the basename */
    return xstrndup(base, length);
}

