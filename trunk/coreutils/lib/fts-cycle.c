/* Detect cycles in file tree walks */

#include "hash.h"
#include "fts-cycle.h"

#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>


#define CC_MAGIC    9828862

/* Return true if I is power of 2, or is zero */
static inline bool
is_zero_or_power_of_two(uintmax_t i)
{
    return (i & (i - 1)) == 0;
}

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


/* Leave a directory during a file tree walk */
void leave_dir(FTS* fts, FTSENT* ent)
{
    struct stat* st = ent->fts_statp;
    if(fts->fts_options & (FTS_TIGHT_CYCLE_CHECK | FTS_LOGICAL))
    {
        struct Active_dir obj;
        void* found;
        obj.dev = st->st_dev;
        obj.ino = st->st_ino;
        found = hash_delete(fts->fts_cycle.ht, &obj);
        if(!found)
            abort();
        free(found);
    }
    else
    {
        FTSENT* parent = ent->fts_parent;
        if(parent != NULL && parent->fts_level >= 0)
            CYCLE_CHECK_REFLECT_CHDIR_UP(fts->fts_cycle.state,
                                          *(parent->fts_statp), *st);
    }
}


/* In traversing a directory hierarchy, call this function once for each
   descending chdir call, with SB corresponding to the chdir operand.
   If SB corresponds to a directory that has already been seen,
   return true to indicate that there is a directory cycle.
   Note that this is done `lazily', which means that some of
   the directories in the cycle may be processed twice before
   the cycle is detected */
bool cycle_check(struct cycle_check_state* state, struct stat* sb)
{
    assert(state->magic == CC_MAGIC);

    /* If the current directory ever happens to be the same
       as the one we last recorded for the cycle detection,
       then it's obviously part of a cycle */
    if(state->chdir_counter && SAME_INODE(*sb, state->dev_ino))
        return true;

     /* If the number of `descending' chdir calls is a power of two,
        record the dev/ino of the current directory */
     if(is_zero_or_power_of_two(++(state->chdir_counter)))
     {
         /* On all architectures that we know about, if the counter
            overflows then there is a directory cycle here somewhere,
            even if we haven't detected it yet. Typically this happens
            only after the counter is incremented 2**64 times, so it's a
            fairly theoretical point */
         if(state->chdir_counter == 0)
             return true;

         state->dev_ino.st_dev = sb->st_dev;
         state->dev_ino.st_ino = sb->st_ino;
     }

     return false;
}


/* Enter a directory during a file tree walk */
bool enter_dir(FTS* fts, FTSENT* ent)
{
    if(fts->fts_options & (FTS_TIGHT_CYCLE_CHECK | FTS_LOGICAL))
    {
        struct stat* st = ent->fts_statp;
        struct Active_dir* ad = malloc(sizeof *ad);
        struct Active_dir *ad_from_table;

        if(!ad)
            return false;

        ad->dev = st->st_dev;
        ad->ino = st->st_ino;
        ad->fts_ent = ent;

        /* See if we've already encountered this directory.
           This can happen when following symlinks as well as
           with a corrupted directory hierarchy */
        ad_from_table = hash_insert(fts->fts_cycle.ht, ad);

        if(ad_from_table != ad)
        {
            free(ad);
            if(!ad_from_table)
                return false;

            /* There was an entry with matching dev/inode already in the table.
               Record the fact that we've found a cycle */
            ent->fts_cycle = ad_from_table->fts_ent;
            ent->fts_info = FTS_DC;
        }
    }
    else
    {
        if(cycle_check(fts->fts_cycle.state, ent->fts_statp))
        {
            ent->fts_cycle = ent;
            ent->fts_info = FTS_DC;
        }
    }
    return true;
}
