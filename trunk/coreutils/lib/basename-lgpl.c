//#include <config.h>
#include <stdbool.h>  // for type bool
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
