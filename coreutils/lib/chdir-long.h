/* provide a chdir function that tries not to fail due to ENAMETOOLONG */
#ifndef CHDIR_LONG_H
#define CHDIR_LONG_H

#include <unistd.h>
#include <limits.h>

#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# endif
#endif


#endif
