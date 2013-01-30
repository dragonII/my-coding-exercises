/* human.h -- print human readable file size */

#ifndef HUMAN_H
#define HUMAN_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "xstrtol.h"

/* A conservative bound on the maximum length of a human-readable string.
   The output can be the square of the largest uintmax_t, so double
   its size before converting to a bound;
   log10 (2.0) < 146/485. Add 1 for integer division truncation.
   Also, the output can have a thousands separator between every digit,
   so multiply by MB_LEN_MAX + 1 and then subtract MB_LEN_MAX.
   Append 1 for a space before the suffix.
   Finally, append 3, the maximum length of a suffix */
# define LONGEST_HUMAN_READABLE \
   ((2 * sizeof (uintmax_t) * CHAR_BIT * 146 / 485 + 1) * (MB_LEN_MAX + 1) \
    - MB_LEN_MAX + 1 + 3)

char* human_readable(uintmax_t n, char* buf, int opts,
                     uintmax_t from_block_size, uintmax_t to_block_size);


/* Options for human_readable */
enum
{
    /* Unless otherwise specified these options may be ORed together */

    /* The following three options are mutually exclusive. */
    /* Round to plus infinity (default). */
    human_ceiling = 0,
    /* Round to nearest, ties to even. */
    human_round_to_nearest = 1,
    /* Round to minus infinity */
    human_floor = 2,

    /* Group digits together, e.g., `1,000,000'. This uses the
       locale-defined grouping; the traditional C locale does not group,
       so this has effect only if some other locale is in use. */
    human_group_digits = 4,

    /* When autoscaling, suppress ".0" at end. */
    human_suppress_point_zero = 8,

    /* Scale output and use SI-style units, ignoring the output block size. */
    human_autoscale = 16,

    /* Prefer base 1024 to base 1000 */
    human_base_1024 = 32,

    /* Prepend " " before unit symbol */
    human_space_before_unit = 64,

    /* Append SI prefixx, e.g. "k" or "M" */
    human_SI = 128,

    /* Append "B" (if base 1000) or "iB" (if base 1024) to SI prefix */
    human_B = 256
};


#endif
