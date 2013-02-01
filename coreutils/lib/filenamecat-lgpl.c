/* Concatenate two arbitrary file names */

#include "filenamecat.h"

#include <stdlib.h>
#include <string.h>

#include "dirname.h"
#include "basename-lgpl.h"

/* Return the longest suffix of F that is a relative file name.
   If it has no such suffix, return the empty string. */
static char* longest_relative_suffix(char* f)
{
    for(f += FILE_SYSTEM_PREFIX_LEN(f); ISSLASH(*f); f++)
        continue;
    return f;
}

/* Concatenate two file name components, DIR and ABASE, in
   newly-allocated storage and return the result.
   The resulting file name F is such that the commands "ls F" and "(cd
   DIR; ls BASE)" refer to the same file, where BASE is ABASE with any
   file system prefixes and leading separators removed.
   Arrange for a directory separator if necessary between DIR and BASE
   in the result, removing any redundant separators.
   In any cases, if BASE_IN_RESULT is non-NULL, set
   *BASE_IN_RESULT to point to the copy of ABASE in the returned
   concatenation. However, if ABASE begins with more than on slash,
   set *BASE_IN_RESULT to point to the sole corresponding slash that
   is copied into the result buffer.

   Return NULL if malloc fails */

char* mfile_name_concat(char* dir, char* abase, char** base_in_result)
{
    char* dirbase = last_component(dir);
    size_t dirbaselen = base_len(dirbase);
    size_t dirlen = dirbase - dir + dirbaselen;
    size_t needs_separator = (dirbaselen && ! ISSLASH(dirbase[dirbaselen - 1]));

    char* base = longest_relative_suffix(abase);
    size_t baselen = strlen(base);

    char* p_concat = malloc(dirlen + needs_separator + baselen + 1);
    char* p;

    if(p_concat == NULL)
        return NULL;

    p = mempcpy(p_concat, dir, dirlen);
    *p = DIRECTORY_SEPARATOR;
    p += needs_separator;

    if(base_in_result)
        *base_in_result = p - IS_ABSOLUTE_FILE_NAME(abase);

    p = mempcpy(p, base, baselen);
    *p = '\0';

    return p_concat;
}
