/* fpending.c - return the number of pending output bytes on a stream */

#include "fpending.h"

/* Return the number of pending (aka buffered, unflushed)
   bytes on the stream, FP, that is open for writing. */

size_t __fpending(FILE* fp)
{
    return PENDING_OUTPUT_N_BYTES;
}
