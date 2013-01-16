/* hash - hashing table processing */

#include "hash.h"


/* Allocate and return a new hash table, or NULL upon failure. The initial
   number of buckets is automatically selected so as to _guarantee_ that you
   may insert at least CANDIDATE different user entries before any growth of
   the hash table size occurs. So, if have a reasonably tight a-priori upper
   bound on the number of entries you intend to insert in the hash table, you
   may save some table memory and insertion time, by specifying it here. If
   the IS_N_BUCKETS field of the TUNING structure is true, the CANDIDATE
   argument has its meaning changed to the wanted number of buckets.

   TUNING points to a structure of user-supplied values, in case some fine
   tuning is wanted over the default behavior of the hasher. If TUNING is
   NULL, the default tuning parameters are used instead. If TUNING is
   provided but the values requested are out of bounds or might cause 
   rounding errors, return NULL.

   The user-supplied HASHER function, when not NULL, accepts two
   arguments ENTRY and TABLE_SIZE. It computes, by hashing ENTRY contents, a
   slot number for that entry which should be in the range 0..TABLE_SIZE-1.
   This slot number is then returned.

   The user-supplied COMPARATOR function, when not NULL, accepts two
   arguments pointing to user data, it then returns true for a pair of entries
   that compare equal, or false otherwise. This function is internally called
   on entries which are already known to hash to the same buchet index,
   but which are distinct pointers.

   The user-supplied DATA_FREER function, when not NULL, may be later called
   with the user data as an argument, just before the entry containing the
   data gets freed. This happens from within `hash_free' or `hash_clear'.
   You should specify this function only if you want these functions to free
   all of your `data' data. This is typically the case when your data is
   simply an auxilary struct that you have malloc'd to aggregate several
   values. */
Hash_table* hash_initialize(size_t candidate, const Hash_tuning* tuning,
                            Hash_hasher hasher, Hash_comparator comparator,
                            Hash_data_freer data_freer)
{
    Hash_table* table;

    if(hasher == NULL)
        hasher = raw_hasher;
    if(comparator == NULL)
        comparator = raw_comparator;

    table = malloc(sizeof *table)
    if(table == NULL)
        return NULL;

    if(!tuning)
        tuning = &default_tuning;

    table->tuning = tuning;
    if(!check_tuning(table))
    {
        /* Fail if the tuning options are invalid. This is the only occasion
           when the user gets some feedback about it. Once the table is created,
           if the user provides invalid tuning options, we silently revert to
           using the defaults, and ignore further request to change the tuning
           options. */
           goto fail;
    }

    table->n_buckets = compute_bucket_size(candidate, tuning);
    if(!table->n_buckets)
        goto fail;

    table->buckets = calloc(table->n_buckets, sizeof *table->buckets);
    if(table->buckets == NULL)
        goto fail;

    table->bucket_limit = table->buckets + table->n_buckets;
    table->n_buckets_used = 0;
    table->n_entries = 0;

    table->hasher = hasher;
    table->comparator = comparator;
    table->data_freer = data_freer;

    table->free_entry_list = NULL;

    return table;

fail:
    free(table);
    return NULL;
}
