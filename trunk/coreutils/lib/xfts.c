/* xfts,c -- a wrapper for fts_open */

#include "xalloc.h"
#include "xfts.h"

#include <assert.h>
#include <errno.h>

/* Fail with a proper diagnostic if fts_open fails */
FTS*
xfts_open(char** argv, int options,
          int (*compar)(FTSENT**, FTSENT**))
{
    FTS* fts = fts_open(argv, options | FTS_CWDFD, compar);
    if(fts == NULL)
    {
        /* This can fail in two ways: out of memory or with errno == EINVAL,
           which indicates it was called with invalid bit_flags */
        assert(errno != EINVAL);
        xalloc_die();
    }

    return fts;
}

/* When fts_read returns FTS_DC to indicate a directory cycle,
   it may or may not indicate a real problem. When a program like
   chgrp performs a recursive traversal that requires traversing
   symbolic links, it is *not* a problem. However, when invoked
   with "-P -R", it deserves a warning. The fts_options member
   records the options that control this aspect of fts's behavior,
   so test that. */
bool cycle_warning_required(FTS* fts, FTSENT* ent)
{
#define ISSET(Fts, Opt) ((Fts)->fts_options & (Opt))
    /* When dereferencing no symlinks, or when dereferencing only
       those listed on the command line and we're not processing
       a command-line argument, then a cycle is a serious problem. */
    return ((ISSET(fts, FTS_PHYSICAL) && !ISSET(fts, FTS_COMFOLLOW))
            || (ISSET(fts, FTS_PHYSICAL) && ISSET(fts, FTS_COMFOLLOW)
                && ent->fts_level != FTS_ROOTLEVEL));
}
