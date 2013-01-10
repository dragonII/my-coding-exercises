#include <stdbool.h>  // for type bool
#include <stdlib.h>
#include <string.h>

#include "dirname.h"

/*
 * Return the address of the last file name component of NAME. If
 * NAME has no relative file name component because it is a file
 * system root, return the empty string.
 */
char* last_component(char const* name)
{
    char const* base = name + FILE_SYSTEM_PREFIX_LEN(name);
    char const* p;
    bool saw_slash = false;

    while(ISSLASH(*base))
        base++;

    for(p = base; *p; p++)
    {
        if(ISSLASH(*p))
            saw_slash = true;
        else if(saw_slash)
        {
            base = p;
            saw_slash = false;
        }
    }
    return (char*)base;
}


/* Return the length of the basename NAME. Typically NAME is the
   value returned by base_name or last_component. Act like strlen
   (NAME), except omit all trailing slashes. */
size_t base_len(char* name)
{
    size_t len;
    size_t prefix_len = FILE_SYSTEM_PREFIX_LEN(name);

    /* strip trailing slashes */
    for(len = strlen(name); len > 1 && ISSLASH(name[len - 1]); len--)
        continue;

    if(DOUBLE_SLASH_IS_DISTINCT_ROOT && len == 1
        && ISSLASH(name[0]) && ISSLASH(name[1]) && ! name[2])
        return 2;

    if(FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE && prefix_len
        && len == prefix_len && ISSLASH(name[prefix_len]))
        return prefix_len + 1;

    return len;
}
