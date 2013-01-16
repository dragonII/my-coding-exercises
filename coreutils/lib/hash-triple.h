#ifndef HASH_TRIPLE_H
#define HASH_TRIPLE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

/* Describe a just-created or just-renamed destination file */
struct F_triple
{
    char* name;
    ino_t st_ino;
    dev_t st_dev;
};

size_t triple_hash(void* x, size_t table_size);
bool triple_compare_ino_str(void* x, void* y);
void triple_free(void* x);


#endif
