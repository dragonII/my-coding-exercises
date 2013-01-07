/* dirname.c -- return all but the last element in a file name */

#include "dirname.h"

#include <stdlib.h>
#include <string.h>

/* Return the length of the prefix of FILE that will be used by
   dir_name. If FILE is in the working directory, this returns zero
   even though `dir_name(FILE)' will return ".". Works properly even
   if there are trailing slashes (by effectively ignoring them). */

size_t dir_len(char* file)
{
    size_t prefix_length = FILE_SYSTEM_PREFIX_LEN(file);
    size_t length;

    /* Advance prefix_length beyond important leading slashes. */
