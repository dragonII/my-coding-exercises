/* Generate random integers */

#include "randint.h"

#include <sys/types.h>
#include <errno.h>

#include "randread.h"
#include "xalloc.h"

struct randint_source*
randint_new(struct randread_source* source)
{
    struct randint_source* s = xmalloc(sizeof *s);
    s->source = source;
    s->randnum = s->randmax = 0;
    return s;
}


/* Create a new randint_source by creating a randread_source from
   NAME and ESTIMATED_BYTES. Return NULL (setting errno) if
   unsucessful */
struct randint_source*
randint_all_new(char* name, size_t bytes_bound)
{
    struct randread_source *source = randread_new(name, bytes_bound);
    return (source ? randint_new(source) : NULL);
}
