/* Generate random integers */

#ifndef RANDINT_H
# define RANDINT_H

#include <sys/types.h>
#include <stdint.h>

/* An unsigned integer type, used for random integers, and its maximum
   value */
typedef uintmax_t randint;
# define RANDINT_MAX UINTMAX_MAX

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


struct randint_source*
randint_all_new(char* name, size_t bytes_bound);
randint randint_genmax(struct randint_source* s, randint genmax);
void randint_free(struct randint_source* s);
int randint_all_free(struct randint_source* s);

#endif
