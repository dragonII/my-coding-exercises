/* Detect cycles in file tree walks */

#include "hash.h"
#include "fts-cycle.h"

#include <stdlib.h>
#include <inttypes.h>

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


/* Use each of these to map a device/inode pair to an FTSENT */
struct Active_dir
{
    dev_t dev;
    ino_t ino;
    FTSENT* fts_ent;
};


static bool AD_compare(void* x, void* y)
{
    struct Active_dir* ax = x;
    struct Active_dir* ay = y;
    return ax->ino == ay->ino && ax->dev == ay->dev;
}

static size_t
AD_hash(void* x, size_t table_size)
{
    struct Active_dir* ax = x;
    return (uintmax_t)ax->ino % table_size;
}

/* Set up the cycle-detection machinery */
bool setup_dir(FTS* fts)
{
    if(fts->fts_options & (FTS_TIGHT_CYCLE_CHECK | FTS_LOGICAL))
    {
        enum { HT_INITIAL_SIZE = 31 };
        fts->fts_cycle.ht = hash_initialize(HT_INITIAL_SIZE, NULL, AD_hash,
                                            AD_compare, free);
        if(! fts->fts_cycle.ht)
            return false;
    }
    else
    {
        fts->fts_cycle.state = malloc(sizeof *fts->fts_cycle.state);
        if(! fts->fts_cycle.state)
            return false;
        cycle_check_init(fts->fts_cycle.state);
    }

    return true;
}
