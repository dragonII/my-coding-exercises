/* Hash functions for file-related triples: name, device, inode. */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "hash-triple.h"
#include "hash-pjw.h"
#include "same-inode.h"

#define STREQ(a, b) (strcmp ((a), (b)) == 0)

/* Hash an F_triple, and *d* consider the file name */
size_t triple_hash(void* x, size_t table_size)
{
    struct F_triple* p = x;
    size_t tmp = hash_pjw(p->name, table_size);

    /* Ignoring the device number here should be fine */
    return (tmp ^ p->st_ino) % table_size;
}

bool triple_compare_ino_str(void* x, void* y)
{
    struct F_triple* a = x;
    struct F_triple* b = y;
    return (SAME_INODE(*a, *b) && STREQ(a->name, b->name)) ? true : false;
}

/* Free an F_triple */
void triple_free(void* x)
{
    struct F_triple* a = x;
    free(a->name);
    free(a);
}
