/* hash - hashing table processing. */

#ifndef HASH_H
#define HASH_H

#include <sys/types.h>
#include <stdbool.h>

typedef size_t (*Hash_hasher) (void*, size_t);
typedef bool (*Hash_comparator)(void*, void*);
typedef void (*Hash_data_freer)(void*);

struct hash_tuning
{
    /* This structure is mainly used for `hash_initialize' */
    float shrink_threshold;     /* ratio of used buckets to trigger a shrink */
    float shrink_factor;        /* ratio of new smaller size to original size */
    float growth_threshold;     /* ratio of used buckets to trigger a growth */
    float growth_factor;        /* ratio of new bigger size to original size */
    bool  is_n_buckets;         /* if CANDIDATE really means table size */
};

typedef struct hash_tuning Hash_tuning;

struct hash_table
{
    /* The array of buckets starts at BUCKET and extends to BUCKET_LIMIT - 1,
       for a possibility of N_BUCKETS. Among those, N_BUCKETS_USED buckets
       are not empty, there are N_ENTRIES active entries in the table. */
    struct hash_entry *bucket;
    struct hash_entry *bucket_limit;
    size_t n_buckets;
    size_t n_buckets_used;
    size_t n_entries;

    /* Tuning arguments, kept in a physically separate structure */
    const Hash_tuning* tuning;

    /* These functions are given to `hash_initialize'.
       In a word, HASHER randomizes a user entry into a number
       up from 0 to some maximum minus 1; COMPARATOR returns
       true if two user entries compare equally; and DATA_FREER is
       the cleanup function for a user entry */
    Hash_hasher hasher;
    Hash_comparator comparator;
    Hash_data_freer data_freer;

    /* A linked list of freed struct hash_entry structs */
    struct hash_entry* free_entry_list;
};

/* A hash table contains many internal entries, each holding a pointer to
   some user-provided data (also called a user entry). An entry indistinctly
   refers to both the internal entry and its associated user entry. A user
   entry contents may be hashed by a randomization function (the hashing
   function, or just `hash' for short) into a number (or `slot') between 0
   and the current table size. At each slot position in the hash table,
   starts a linked chain of entries for which the user data all hash to this
   slot. A bucket is the collection of all entries hashing to the same slot. */


typedef struct hash_table Hash_table;

Hash_table* hash_initialize(size_t candidate, const Hash_tuning* tuning,
                            Hash_hasher hasher, Hash_comparator comparator,
                            Hash_data_freer data_freer);


#endif
