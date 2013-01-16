#ifndef FILE_SET_H
#define FILE_SET_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "hash.h"

void record_file(Hash_table* ht, char* file, struct stat* stats);
bool seen_file(Hash_table* ht, char* file, struct stat* stats);


#endif
