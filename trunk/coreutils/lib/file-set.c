/* Specialized functions to manipulate a set of files */

#include <stdbool.h>
#include <sys/stat.h>

#include "hash.h"
#include "hash-triple.h"
#include "xalloc.h"

/* Record file, FILE, and dev/ino from *STAT, in the hash table, HT.
   If HT is NULL, return immediately.
   If memory allocation failed, exit immediately */
void record_file(Hash_table* ht, char* file, struct stat* stats)
{
    struct F_triple* ent;

    if(ht == NULL)
        return;

    ent = xmalloc(sizeof *ent);
    ent->name = xstrdup(file);
    ent->st_ino = stats->st_ino;
    ent->st_dev = stats->st_dev;

    {
        struct F_triple* ent_from_table = hash_insert(ht, ent);
        if(ent_from_table == NULL)
        {
            /* Insertion failed due to lack of memory */
            xalloc_die();
        }

        if(ent_from_table != ent)
        {
            /* There was already a matching entry in the table, so ENT was
               not inserted. Free it */
            triple_free(ent);
        }
    }
}

/* Return true if there is an entry in hash table, HT,
   for the file described by FILE and STATS. */
bool seen_file(Hash_table* ht, char* file, struct stat* stats)
{
    struct F_triple new_ent;

    if(ht == NULL)
        return false;

    new_ent.name = (char*)file;
    new_ent.st_ino = stats->st_ino;
    new_ent.st_dev = stats->st_dev;

    return !!hash_lookup(ht, &new_ent);
}
