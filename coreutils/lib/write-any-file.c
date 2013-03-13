/* Determine whether we can write any file */

#include "write-any-file.h"
#include "priv-set.h"

#include <unistd.h>

/* Return true if we know that we can write any file, including
   writing directories */
bool can_write_any_file(void)
{
    static bool initialized;
    static bool can_write;

    if(!initialized)
    {
        bool can = false;

        can = (geteuid() == 0);

        can_write = can;
        initialized = true;
    }

    return can_write;
}
