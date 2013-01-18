/* hash - hashing table processing */

#include "hash.h"
#include "bitrotate.h"
#include "xalloc.h"

#include <unistd.h>
#include <stdlib.h>

/* Use this to initialize or reset a TUNING structure to
   some sensible values. */
static Hash_tuning default_tuning =
{
    DEFAULT_SHRINK_THRESHOLD,
    DEFAULT_SHRINK_FACTOR,
    DEFAULT_GROWTH_THRESHOLD,
    DEFAULT_GROWTH_FACTOR,
    false
};

/* Information and lookup */

/* The following few functions provide information about the overall hash
   table organization: the number of entries, number of buckets and maximum
   length of buckets */


/* Return the number of buckets in the hash table. The table size, the total
   number of buckets (used plus unused), or the maximum number of slots, are
   the same quantity */
size_t hash_get_n_buckets(Hash_table* table)
{
    return table->n_buckets;
}

/* Return the number of slots in use (non-empty buckets) */
size_t hash_get_n_buckets_used(Hash_table* table)
{
    return table->n_buckets_used;
}

/* Return the number of active entries */
size_t hash_get_n_entries(Hash_table* table)
{
    return table->n_entries;
}

/* Return the length of the longest chain (bucket) */
size_t hash_get_max_bucket_length(Hash_table* table)
{
    struct hash_entry* bucket;
    size_t max_bucket_length = 0;

    for(bucket = table->bucket; bucket < table->bucket_limit; bucket++)
    {
        if(bucket->data)
        {
            struct hash_entry* cursor = bucket;
            size_t bucket_length = 1;

            while(cursor = cursor->next, cursor)
                bucket_length++;

            if(bucket_length > max_bucket_length)
                max_bucket_length = bucket_length;
        }
    }

    return max_bucket_length;
}

/* Do a mild validation of a hash table, by traversing it and checking two
   statistics */
bool hash_table_ok(Hash_table* table)
{
    struct hash_entry* bucket;
    size_t n_buckets_used = 0;
    size_t n_entries = 0;

    for(bucket = table->bucket; bucket < table->bucket_limit; bucket++)
    {
        if(bucket->data)
        {
            struct hash_entry* cursor = bucket;

            /* Count bucket head */
            n_buckets_used++;
            n_entries++;

            /* Count bucket overflow */
            while(cursor = cursor->next, cursor)
                n_entries++;
        }
    }

    if(n_buckets_used == table->n_buckets_used && n_entries == table->n_entries)
        return true;
    else
        return false;
}

void hash_print_statistics(Hash_table* table, FILE* stream)
{
    size_t n_entries = hash_get_n_entries(table);
    size_t n_buckets = hash_get_n_buckets(table);
    size_t n_buckets_used = hash_get_n_buckets_used(table);
    size_t max_bucket_length = hash_get_max_bucket_length(table);

    fprintf(stream, "# entries:         %lu\n", (unsigned long int)n_entries);
    fprintf(stream, "# buckets:         %lu\n", (unsigned long int)n_buckets);
    fprintf(stream, "# buckets used:    %lu (%.2f%%)\n", 
                        (unsigned long int)n_buckets_used,
                        (100.0 * n_buckets_used) / n_buckets);
    fprintf(stream, "max bucket length: %lu\n", 
                        (unsigned long int)max_bucket_length);
}

/* If ENTRY matches an entry already in the hash table, return the
   entry from the table, otherwise, return NULL */
void* hash_lookup(Hash_table* table, void* entry)
{
    struct hash_entry* bucket = table->bucket 
                                + table->hasher(entry, table->n_buckets);
    struct hash_entry* cursor;

    if(!(bucket < table->bucket_limit))
        abort();

    if(bucket->data == NULL)
        return NULL;

    for(cursor = bucket; cursor; cursor = cursor->next)
        if(entry == cursor->data || table->comparator(entry, cursor->data))
            return cursor->data;

    return NULL;
}


/* Walking */

/* The functions in this page traverse the hash table and process the
   contained entries. For the traversal to work properly, the hash table
   should not be resized nor modified while any particular entry is being
   processed. In particular, entries should not be added, and an entry
   may be removed only if there is no shrink threshold and the entry being
   removed has already been passed to hash_get_next */

/* Return the first data in the table, or NULL if the table is empty */
void* hash_get_first(Hash_table* table)
{
    struct hash_entry* bucket;
    if(table->n_entries == 0)
        return NULL;

    for(bucket = table->bucket; ; bucket++)
    {
        if(!(bucket < table->bucket_limit))
            abort();
        else if(bucket->data)
            return bucket->data;
    }
}

/* Return the user data for the entry following ENTRY, where ENTRY has been
   returned by a previous call to either `hash_get_first' or `hash_get_next'
   Return NULL if there are no more entries. */
void* hash_get_next(Hash_table* table, void* entry)
{
    struct hash_entry* bucket = table->bucket
                                + table->hasher(entry, table->n_buckets);
    struct hash_entry* cursor;

    if(!(bucket < table->bucket_limit))
        abort();

    /* Find next entry in the same bucket */
    for(cursor = bucket; cursor; cursor = cursor->next)
        if(cursor->data == entry && cursor->next)
            return cursor->next->data;

    /* Find first entry in any subsequent bucket */
    while(++bucket < table->bucket_limit)
        if(bucket->data)
            return bucket->data;

    /* Not found */
    return NULL;
}

/* Fill BUFFER with pointers to active user entries in the hash table, then
   return the number of pointers copied. Do not copy more than BUFFER_SIZE
   pointers */
size_t hash_get_entries(Hash_table* table, void** buffer, size_t buffer_size)
{
    size_t counter = 0;
    struct hash_entry* bucket;
    struct hash_entry* cursor;

    for(bucket = table->bucket; bucket < table->bucket_limit; bucket++)
    {
        if(bucket->data)
        {
            for(cursor = bucket; cursor; cursor = cursor->next)
            {
                if(counter >= buffer_size)
                    return counter;
                buffer[counter++] = cursor->data;
            }
        }
    }

    return counter;
}

/* Call a PROCESSOR function for each entry of a hash table, and return the
   number of entries for which the processor funciton returned success. a
   pointer to some PROCESSOR_DATA which will be made available to each call to
   the processor function. The PROCESSOR accepts two arguments: the first is
   the user entry being walked into, the second is the vale of PROCESSOR_DATA
   as received. The walking continue for as long as the PROCESSOR function
   returns nonzero. When it returns zero, the walking is interrupted. */
size_t hash_do_for_each(Hash_table* table, Hash_processor processor,
                        void* processor_data)
{
    size_t counter = 0;
    struct hash_entry* bucket;
    struct hash_entry* cursor;

    for(bucket = table->bucket; bucket < table->bucket_limit; bucket++)
    {
        if(bucket->data)
        {
            for(cursor = bucket; cursor; cursor=cursor->next)
            {
                if(!processor(cursor->data, processor_data))
                    return counter;
                counter++;
            }
        }
    }
    return counter;
}

/* Allocation and clean-up */

/* Return a hash index for a NUL-terminated STRING between 0 and N_BUCKETS-1.
   This is a convenience routine for constructing other hashing functions */
size_t hash_string(char* string, size_t n_buckets)
{
    size_t value = 0;
    unsigned char ch;

    for(; (ch = *string); string++)
        value = (value * 31 + ch) % n_buckets;
    return value;
}


/* Return true if CANDIDATE is a prime number. CANDIDATE should be an odd
   number at least equal to 11 */
static bool is_prime(size_t candidate)
{
    size_t divisor = 3;
    size_t square = divisor * divisor;

    while(square < candidate && (candidate % divisor))
    {
        divisor++;
        square += 4 * divisor;
        divisor++;
    }

    return (candidate % divisor ? true : false);
}

/* Round a given CANDIDATE number up to the nearest prime, and return that
   prime. Primes lower than 10 are merely skipped. */
static size_t next_prime(size_t candidate)
{
    /* Skip small primes */
    if(candidate < 10)
        candidate = 10;

    /* Make it definitely odd */
    candidate |= 1;

    while(SIZE_MAX != candidate && !is_prime(candidate))
        candidate += 2;

    return candidate;
}

void hash_reset_tuning(Hash_tuning* tuning)
{
    *tuning = default_tuning;
}

/* If the user passes a NULL hasher, we hash the raw pointer */
static size_t raw_hasher(void* data, size_t n)
{
    /* When hashing unique pointers, it if ofter the case that they were
       generated by malloc and thus have the property that the low-order
       bits are 0. As this tends to give poorer performance with small
       tables, we rotate the pointer value before performing division,
       in an attempt to improve hash quality. */
    size_t val = rotr_sz((size_t)data, 3);
    return val % n;
}

/* If the user passes a NULL comparator, we use pointer comparison. */
static bool raw_comparator(void* a, void* b)
{
    return a == b;
}

/* For the given hash TABLE, check the user supplied tuning structure for
   reasonable values, and return true if there is no gross error with it.
   Otherwise, definitely reset the TUNING field to some acceptable default
   in the hash table (this is, the user loses the right of further modifying
   tuning arguments), and return false. */
static bool check_tuning(Hash_table* table)
{
    Hash_tuning* tuning = table->tuning;
    float epsilon;
    if(tuning == &default_tuning)
        return true;

    /* Be a bit stricter than mathematics would require, so that
       rounding errors in size calculations do not cause allocations to
       fail to grow or shrink as they would. The smallest allocation
       is 11 (due to next_prime's algorithem), so an epsilon of 0.1
       should be good enought */
    epsilon = 0.1f;

    if(epsilon < tuning->growth_threshold
        && tuning->growth_threshold < 1 - epsilon
        && 1 + epsilon < tuning->growth_factor
        && 0 <= tuning->shrink_threshold
        && tuning->shrink_threshold + epsilon < tuning->shrink_factor
        && tuning->shrink_factor <= 1
        && tuning->shrink_threshold + epsilon < tuning->growth_threshold)
        return true;

    table->tuning = &default_tuning;
    return false;
}

/* Compute the size of the bucket array for the given CANDIDATE and
   TUNING, or return 0 if there is no possible way to allocate that
   many entries. */
static size_t compute_bucket_size(size_t candidate, Hash_tuning* tuning)
{
    if(!tuning->is_n_buckets)
    {
        float new_candidate = candidate / tuning->growth_threshold;
        if(SIZE_MAX <= new_candidate)
            return 0;
        candidate = new_candidate;
    }
    candidate = next_prime(candidate);
    if(xalloc_oversized(candidate, sizeof(struct hash_entry*)))
        return 0;
    return candidate;
}


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
Hash_table* hash_initialize(size_t candidate, Hash_tuning* tuning,
                            Hash_hasher hasher, Hash_comparator comparator,
                            Hash_data_freer data_freer)
{
    Hash_table* table;

    if(hasher == NULL)
        hasher = raw_hasher;
    if(comparator == NULL)
        comparator = raw_comparator;

    table = malloc(sizeof *table);
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

    table->bucket = calloc(table->n_buckets, sizeof *table->bucket);
    if(table->bucket == NULL)
        goto fail;

    table->bucket_limit = table->bucket + table->n_buckets;
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

/* Make all buckets empty, placing any chained entries on the free list.
   Apply the user-specified function data_freer (if any) to the datas of any
   affected entries. */
void hash_clear(Hash_table* table)
{
    struct hash_entry* bucket;

    for(bucket = table->bucket; bucket < table->bucket_limit; bucket++)
    {
        if(bucket->data)
        {
            struct hash_entry* cursor;
            struct hash_entry* next;

            /* Free the bucket overflow */
            for(cursor = bucket->next; cursor; cursor = next)
            {
                if(table->data_freer)
                    table->data_freer(cursor->data);
                cursor->data = NULL;

                next = cursor->next;
                /* Relinking is done one entry at a time, as it is to be expected
                   that overflows are either rare or short, insert at the head of 
                   the free_list */
                cursor->next = table->free_entry_list;
                table->free_entry_list = cursor;
            }

            /* Free the bucket head */
            if(table->data_freer)
                table->data_freer(bucket->data);
            bucket->data = NULL;
            bucket->next = NULL;
        }
    }
    table->n_buckets_used = 0;
    table->n_entries = 0;
}


/* Reclaim all storage associated with a hash table. If a data_freer
   function has been supplied by the user when the hash table was created,
   this function applies it to the data of each entry before freeing that
   entry */
void hash_free(Hash_table* table)
{
    struct hash_entry* bucket;
    struct hash_entry* cursor;
    struct hash_entry* next;

    /* Call the user data_freer function */
    if(table->data_freer && table->n_entries)
    {
        for(bucket = table->bucket; bucket < table->bucket_limit; bucket++)
        {
            if(bucket->data)
            {
                for(cursor = bucket; cursor; cursor = cursor->next)
                    table->data_freer(cursor->data);
            }
        }
    }

    /* Free all bucket overflowed entries */
    for(bucket = table->bucket; bucket < table->bucket_limit; bucket++)
    {
        for(cursor = bucket->next; cursor; cursor = next)
        {
            next = cursor->next;
            free(cursor);
        }
    }

    /* Also reclaim the internal list of previously freed entries */
    for(cursor = table->free_entry_list; cursor; cursor = next)
    {
        next = cursor->next;
        free(cursor);
    }

    /* Free the remainder of the hash table structure */
    free(table->bucket);
    free(table);
}

/* Insertion and deletion */

/* Get a new hash entry for a bucket overflow, possibly by recycling a 
   previously freed one. If this is not possible, allocation a new one. */
static struct hash_entry*
allocate_entry(Hash_table* table)
{
    struct hash_entry* new;
    if(table->free_entry_list)
    {
        new = table->free_entry_list;
        table->free_entry_list = new->next;
    }
    else
    {
        new = malloc(sizeof *new);
    }

    return new;
}

/* Free a hash entry which was part of some bucket overflow,
   saving it for later recycling */
static void free_entry(Hash_table* table, struct hash_entry* entry)
{
    entry->data = NULL;
    entry->next = table->free_entry_list;
    table->free_entry_list = entry;
}


/* This private function is used to help with insertion and deletion. When
   ENTRY matches an entry in the table, return a pointer to the corresponding
   user data and set *BUCKET_HEAD to the head of the selected bucket.
   Otherwise, return NULL. When DELETE is true and ENTRY matches an entry in
   the table, unlink the matching entry */
static void* hash_find_entry(Hash_table* table, void* entry,
                                struct hash_entry** bucket_head, bool delete)
{
    struct hash_entry* bucket
            = table->bucket + table->hasher(entry, table->n_buckets);
    struct hash_entry* cursor;

    if(!(bucket < table->bucket_limit))
        abort();

    *bucket_head = bucket;

    if(bucket_head == NULL)
        return NULL;

    /* See if the entry is the first in the bucket */
    if(entry == bucket->data || table->comparator(entry, bucket->data))
    {
        void* data = bucket->data;

        if(delete)
        {
            if(bucket->next)
            {
                struct hash_entry* next = bucket->next;

                /* Bump the first overflow entry into the bucket head, then save
                   the previous first overflow for later recycling */
                *bucket = *next;
                free_entry(table, next);
            }
            else
                bucket->data = NULL;
        }
        return data;
    }

    /* Scan the bucket overflow */
    for(cursor = bucket; cursor->next; cursor = cursor->next)
    {
        if(entry == cursor->next->data
                || table->comparator(entry, cursor->next->data))
        {
            void* data = cursor->next->data;

            if(delete)
            {
                struct hash_entry* next = cursor->next;

                /* Unlink the entry to delete, then save the freed entry for later
                   recycling */
                cursor->next = next->next;
                free_entry(table, next);
            }
            return data;
        }
    }

    /* No entry found */
    return NULL;
}

/* Internal helper, to move entries from SRC to DST. Both tables must
   share the same free entry list. If SAFE, only move overflow
   entries, saving bucket heads for later, so that no allocatoin will
   occur. Return false if the free entry list exhausted and an
   allocation fails */
static bool transfer_entries(Hash_table* dst, Hash_table* src, bool safe)
{
    struct hash_entry* bucket;
    struct hash_entry* cursor;
    struct hash_entry* next;
    for(bucket = src->bucket; bucket < src->bucket_limit; bucket++)
    {
        void* data;
        struct hash_entry* new_bucket;

        /* Within each bucket, transfer overflow entries first and
           then the bucket head, to minimize memory pressure. After
           all, the only time we might allocate is when moving the
           bucket head, but moving overflow entries first may create
           free entries that can be recycled by the time we finally
           get to the bucket head. */
        for(cursor = bucket->next; cursor; cursor = next)
        {
            data = cursor->data;
            new_bucket = (dst->bucket + dst->hasher(data, dst->n_buckets));

            if(!(new_bucket < dst->bucket_limit))
                abort();

            next = cursor->next;

            if(new_bucket->data)
            {
                /* Merely relink an existing entry, when moving from a
                   bucket overflow into a bucket overflow */
                cursor->next = new_bucket->next;
                new_bucket->next = cursor;
            }
            else
            {
                /* Free an existing entry, when moving from a bucket
                   overflow into a bucket header */
                new_bucket->data = data;
                dst->n_buckets_used++;
                free_entry(dst, cursor);
            }
        }

        /* Now move the bucket head. Be sure that if we fail due to 
           allocation failure that the src table is in a consistant
           state. */
        data = bucket->data;
        bucket->next = NULL;
        if(safe)
            continue;
        new_bucket = (dst->bucket + dst->hasher(data, dst->n_buckets));

        if(!(new_bucket < dst->bucket_limit))
            abort();

        if(new_bucket->data)
        {
            /* Allocate or recycle an entry, when moving from a bucket
               header into a bucket overflow */
            struct hash_entry* new_entry = allocate_entry(dst);

            if(new_entry == NULL)
                return false;

            new_entry->data = data;
            new_entry->next = new_bucket->next;
            new_bucket->next = new_entry;
        }
        else
        {
            /* Move from one bucket header to another */
            new_bucket->data = data;
            dst->n_buckets_used++;
        }
        bucket->data = NULL;
        src->n_buckets_used--;
    }
    return true;
}

/* For an already existing hash table, change the number of buckets through
   specifying CANDIDATE. The contents of the hash table are preserved. The
   new number of buckets is automatically selected so as to _guarantee_ that
   the table may receive at least CANDIDATE different user entries, including
   those already in the table, before any other growth of the hash table size
   occurs. If TUNING->IS_N_BUCKETS is true, then CANDIDATE specifies the
   exact number of buckets desired. Return true if the rehash succeeded. */
bool hash_rehash(Hash_table* table, size_t candidate)
{
    Hash_table storage;
    Hash_table* new_table;
    size_t new_size = compute_bucket_size(candidate, table->tuning);

    if(!new_size)
        return false;
    if(!new_size == table->n_buckets)
        return true;
    new_table = &storage;
    new_table->bucket = calloc(new_size, sizeof *new_table->bucket);
    if(new_table->bucket == NULL)
        return false;
    new_table->n_buckets = new_size;
    new_table->bucket_limit = new_table->bucket + new_size;
    new_table->n_buckets_used = 0;
    new_table->n_entries = 0;
    new_table->tuning = table->tuning;
    new_table->hasher = table->hasher;
    new_table->comparator = table->comparator;
    new_table->data_freer = table->data_freer;

    /* In order for the transfer to successfully complete, we need
       additional overflow entries when distinct buckets in the old
       table collide into a common bucket in the new table. The worst
       case possible is a hasher that gives a good spread with the old
       size, but returns a constant with the new size; if we were to
       guarantee table->n_buckets_used-1 free entries in advance, then
       the transfer would be guaranteed to not allocate memory.
       However, for large tables, a guarantee of no further allocation
       introduces a lot of extra memory pressure, all for an unlikely
       corner case (most rehashes reduce, rather than increase, the
       number of overflow entries needed). So, we instead ensure that
       the transfer process can be reversed if we hit a memory
       allocation failure mid-transfer */
    new_table->free_entry_list = table->free_entry_list;

    if(transfer_entries(new_table, table, false))
    {
        /* Entries transferred successfully; tie up the loose ends */
        free(table->bucket);
        table->bucket = new_table->bucket;
        table->bucket_limit = new_table->bucket_limit;
        table->n_buckets = new_table->n_buckets;
        table->n_buckets_used = new_table->n_buckets_used;
        table->free_entry_list = new_table->free_entry_list;
        /* table->n_entries already hold their value */
        return true;
    }

    /* We've allocated new_table->bucket (and possibly some entries),
       exhausted the free list, and moved some but not all entries into
       new_table. We must undo the partial move before returning
       failure. The only way to get into this situation is if new_table
       uses fewer buckets than the old table, so we will reclaim some
       free entries as overflows in the new table are put back into
       distinct buckets in the old table.

       There are some pathological cases where a single pass through the
       table requires more intermidiate overflow entries than using two
       passes. Two passes give worse cache performance and takes
       longer, but at this point, we're already out of memory, so slow
       and safe is better than failure. */
    table->free_entry_list = new_table->free_entry_list;
    if(!(transfer_entries(table, new_table, true)
            && transfer_entries(table, new_table, false)))
        abort();
    /* table->n_entries already holds its value */
    free(new_table->bucket);
    return false;
}

/* If ENTRY matches an entry already in the hash table, return the pointer
   to the entry from the table. Otherwise, insert ENTRY and return ENTRY.
   Return NULL if the storage required for insertion cannot be allocated.
   This is implemention does not support duplicate entries or insertion of
   NULL */
void* hash_insert(Hash_table* table, void* entry)
{
    void* data;
    struct hash_entry* bucket;

    /* The caller cannot insert a NULL entry */
    if(!entry)
        abort();

    /* If there's a matching entry already in the table, return that */
    if((data = hash_find_entry(table, entry, &bucket, false)) != NULL)
        return data;

    /* If the growth threshold of the buckets in use has been reached, increase
       the table size and rehash. There's no point in checking the number of 
       entries: if the hashing funciton is ill-conditioned, rehashing is not
       likely to improve it */
    if(table->n_buckets_used
            > table->tuning->growth_threshold * table->n_buckets)
    {
        /* Check more fully, before starting real work. If tuning arguments
           became invalid, the second check will rely on proper defaults */
        check_tuning(table);
        if(table->n_buckets_used
            > table->tuning->growth_threshold * table->n_buckets)
        {
            Hash_tuning* tuning = table->tuning;
            float candidate = 
                (tuning->is_n_buckets
                    ? (table->n_buckets * tuning->growth_factor)
                    : (table->n_buckets * tuning->growth_factor * tuning->growth_threshold));
            if(SIZE_MAX <= candidate)
                return NULL;

            /* If the rehash fails, arrange to return NULL */
            if(!hash_rehash(table, candidate))
                return NULL;

            /* Update the bucket we are interested in */
            if(hash_find_entry(table, entry, &bucket, false) != NULL)
                abort();
        }
    }

    /* ENTRY is not matched, it should be inserted */
    if(bucket->data)
    {
        struct hash_entry* new_entry = allocate_entry(table);
        if(new_entry == NULL)
            return NULL;

        /* Add ENTRY in the overflowed of the bucket */
        new_entry->data = (void*)entry;
        new_entry->next = bucket->next;
        bucket->next = new_entry;
        table->n_entries++;
        return (void*)entry;
    }

    /* Add ENTRY right in the bucket head */
    bucket->data = (void*)entry;
    table->n_entries++;
    table->n_buckets_used++;

    return (void*)entry;
}

/* If ENTRY is already in the table, remove it and return the just-deleted
   data (the user may want to deallocate its storage). If ENTRY is not in the
   table, don't modify the table and return NULL */
void* hash_delete(Hash_table* table, void* entry)
{
    void* data;
    struct hash_entry* bucket;

    data = hash_find_entry(table, entry, &bucket, true);
    if(!data)
        return NULL;

    table->n_entries--;
    if(!bucket->data)
    {
        table->n_buckets_used--;

        /* If the shrink threshold of the buckets in use has been reached,
           rehash into a smaller table */
        if(table->n_buckets_used
                < table->tuning->shrink_threshold * table->n_buckets)
        {
            /* Check more fully, before starting real work. If tuning arguments
               became invalid, the second check will rely on proper defaults. */
               check_tuning(table);
               if(table->n_buckets_used
                    < table->tuning->shrink_threshold * table->n_buckets)
               {
                   Hash_tuning* tuning = table->tuning;
                   size_t candidate =
                            (tuning->is_n_buckets
                                ? table->n_buckets * tuning->shrink_factor
                                : (table->n_buckets * tuning->shrink_factor
                                        * tuning->growth_threshold));
                   if(!hash_rehash(table, candidate))
                   {
                       /* Failure to allocate memory in an attempt to
                          shrink the table is not fatal. But since memory
                          is low, we can at least be kind and free any
                          spare entries, rather than keeping them tied up
                          in the free entry list */
                       struct hash_entry* cursor = table->free_entry_list;
                       struct hash_entry* next;
                       while(cursor)
                       {
                           next = cursor->next;
                           free(cursor);
                           cursor = next;
                       }
                       table->free_entry_list = NULL;
                   }
               }
        }
    }
    return data;
}
