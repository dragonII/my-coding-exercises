/* hash-pjw.h -- declaration for a simple hash function */

#ifndef HASH_PJW_H
#define HASH_PJW_H

#include <stddef.h>

/* Compute a hash code for a NUL-terminated string starting at X,
   and return the hash code modulo TABLESIZE.
   The result is platform dependent: it depends on the size of the 'size_t'
   type and on the signedness of the 'char' type */

size_t hash_pjw(void* x, size_t tablesize);

#endif
