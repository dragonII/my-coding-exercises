#include <sys/types.h>
#include <errno.h>


/* A source of random data for generating random integers */
struct randint_source
{
    /* The source of random bytes */
    struct randread_source* source;

    /* RANDNUM is a buffered random integer, whose information has not
       yet been delivered to the caller. It is uniformly distributed in
       the range 0 <= RANDNUM <= RANDMAX. If RANDMAX is zero, then
       RANDNUM must be zero (and in some sense it is not really
       "random"). */
    randint randnum;
    randint randmax;
};


/* Create a new randint_source by creating a randread_source from
   NAME and ESTIMATED_BYTES. Return NULL (setting errno) if
   unsucessful */
struct randint_srouce*
randint_all_new(char* name, size_t bytes_bound)
{
    struct randread_source *source = randread_new(name, bytes_bound);
    return (source ? randint_new(source) : NULL);
}
