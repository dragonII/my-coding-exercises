/* Concatenate two arbitrary file names */

#include "filenamecat.h"

#include <stdlib.h>
#include <string.h>

#include "xalloc.h"


/* Just like mfile_name_concat (filename-lgpl.c), except, rather than
   returning NULL upon malloc failure, here, we report the
   "memory exhausted" condition and exit */

char* file_name_concat(char* dir, char* abase, char** base_in_result)
{
    char* p = mfile_name_concat(dir, abase, base_in_result);
    if(p == NULL)
        xalloc_die();
    return p;
}
