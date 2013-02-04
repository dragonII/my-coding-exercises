/* xfts,c -- a wrapper for fts_open */

#include "xalloc.h"
#include "fts.h"

#include <assert.h>
#include <errno.h>

/* Fail with a proper diagnostic if fts_open fails */
FTS*
xfts_open(char** argv, int options,
          int (*compar)(FTSENT**, FTSENT**))
{
    FTS* fts = fts_open(argv, optionss | FTS_CWDFD, compar);
    if(fts == NULL)
    {
        /* This can fail in two ways: out of memory or with errno == EINVAL,
           which indicates it was called with invalid bit_flags */
        assert(errno != EINVAL);
        xalloc_die();
    }

    return fts;
}

