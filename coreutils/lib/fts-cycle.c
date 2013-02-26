/* Detect cycles in file tree walks */

#include "hash.h"
#include "fts-cycle.h"

#include <stdlib.h>

/* Free any memory used for cycle detection */
void free_dir(FTS* sp)
{
    if(sp->fts_options & (FTS_TIGHT_CYCLE_CHECK | FTS_LOGICAL))
    {
        if(sp->fts_cycle.ht)
            hash_free(sp->fts_cycle.ht);
    }
    else
        free(sp->fts_cycle.state);
}
