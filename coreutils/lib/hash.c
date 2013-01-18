/* hash - hashing table processing */

#include "hash.h"
#include "bitrotate.h"

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

    for(bucket = table->bucket; bucket < table->bucket_limit;bucket++)
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
    size_t e_entries = hash_get_n_entries(table);
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
size_t hash_do_for_each(Hash_table* table, Hash_processor prcessor,
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

    for(; ch = *string; string++)
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
Hash_table* hash_initialize(size_t candidate, const Hash_tuning* tuning,
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

}
