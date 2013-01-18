/* hash - hashing table processing. */

#ifndef HASH_H
#define HASH_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>

typedef size_t (*Hash_hasher) (void*, size_t);
typedef bool (*Hash_comparator)(void*, void*);
typedef void (*Hash_data_freer)(void*);
typedef bool (*Hash_processor) (void*, void*);

struct hash_entry
{
    void* data;
    struct hash_entry* next;
};

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
    Hash_tuning* tuning;

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

typedef struct hash_table Hash_table;

/* A hash table contains many internal entries, each holding a pointer to
   some user-provided data (also called a user entry). An entry indistinctly
   refers to both the internal entry and its associated user entry. A user
   entry contents may be hashed by a randomization function (the hashing
   function, or just `hash' for short) into a number (or `slot') between 0
   and the current table size. At each slot position in the hash table,
   starts a linked chain of entries for which the user data all hash to this
   slot. A bucket is the collection of all entries hashing to the same slot.

   A good `hasher' function will distribute entries rather evenly in buckets.
   In the ideal case, the length of each bucket is roughly the number of
   entries divided by the table size. Finding the slot for a data is usually
   done in constant time by the `hasher', and the later finding of a precise
   entry is linear in time with the size of the bucket. Consequently, a
   larger hash table size (this is, a larger number of buckets) is prone to
   yielding shorter chains, *given* the `hasher' function behaves properly.

   Long buckets slow down the lookup algorithm. One might use big hash table
   sizes in hope to reduce the average length of buckets, but this might
   become inordinate, as unused slots in the hash table take some space. The
   best bet is to make sure you are using a good `hasher' function (beware
   that those are not that easy to write! :-), and to use a table size
   larger than the actual number of entries. */

/* If an insertion makes the ratio of nonempty buckets to table size larger
   than the growth threshold (a number between 0.0 and 1.0), then increase
   the table size by multiplying by the growth factor (a number greater than
   1.0). The growth threshold defaults to 0.8, and the growth factor 
   defaults to 1.414, meaning that the table will have double its size
   every second time 80% of the buckets get used. */
#define DEFAULT_GROWTH_THRESHOLD 0.8
#define DEFAULT_GROWTH_FACTOR   1.414

/* If a deletion empties a bucket and causes the ratio of used buckets to
   table size to become smaller than the shrink threshold (a number between
   0.0 and 1.0), then shrink the table by multiplying by the shrink factor
   (a number greater than the shrink threshold but smaller than 1.0). The
   shrink threshold and factor default to 0.0 and 1.0, meaning that the table
   never shrinks */
#define DEFAULT_SHRINK_THRESHOLD 0.0
#define DEFAULT_SHRINK_FACTOR 1.0


/* Information and lookup */
size_t hash_get_n_buckets(Hash_table*);
size_t hash_get_n_buckets_used(Hash_table*);
size_t hash_get_n_entries(Hash_table*);
size_t hash_get_max_bucket_length(Hash_table*);
bool   hash_table_ok(Hash_table*);
void   hash_print_statistics(Hash_table*, FILE*);
void*  hash_lookup(Hash_table*, void*);

/* Walking */
void*  hash_get_first(Hash_table*);
void*  hash_get_next(Hash_table*, void*);
size_t hash_get_entries(Hash_table*, void**, size_t);
size_t hash_do_for_each(Hash_table*, Hash_processor, void*);

/* Allocation and clean-up */
size_t hash_string(char*, size_t);
void   hash_reset_tuning(Hash_tuning*);
Hash_table* hash_initialize(size_t candidate, Hash_tuning* tuning,
                            Hash_hasher hasher, Hash_comparator comparator,
                            Hash_data_freer data_freer);
void   hash_clear(Hash_table*);
void   hash_free(Hash_table*);

/* Insertion and deletion */
bool   hash_rehash(Hash_table*, size_t);
void*  hash_insert(Hash_table*, void*);
void*  hash_delete(Hash_table*, void*);

#endif
