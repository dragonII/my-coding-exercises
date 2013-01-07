/* dirname.c -- return all but the last element in a file name */

#include <stdlib.h>
#include <string.h>

#include "dirname.h"
#include "basename-lgpl.h"

/* Return the length of the prefix of FILE that will be used by
   dir_name. If FILE is in the working directory, this returns zero
   even though `dir_name(FILE)' will return ".". Works properly even
   if there are trailing slashes (by effectively ignoring them). */

size_t dir_len(char* file)
{
    size_t prefix_length = FILE_SYSTEM_PREFIX_LEN(file);
    size_t length;

    /* Advance prefix_length beyond important leading slashes. */
    prefix_length += (prefix_length != 0
                      ? (FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE
                         && ISSLASH(file[prefix_length]))
                      : (ISSLASH(file[0])
                         ? ((DOUBLE_SLASH_IS_DISTINCT_ROOT
                             && ISSLASH(file[1] && !ISSLASH(file[2]))
                             ? 2 : 1))
                         : 0));
    /* prefix_length = 1 for absolute, 0 for relative */

    /* strip the basename and any rdundant slashes before it. */
    for(length = last_component(file) - file;
            prefix_length < length; length--)
        if(!ISSLASH(file[length - 1]))
            break;
     return length;
}
