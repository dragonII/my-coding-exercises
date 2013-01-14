/* intprops.h -- properties of integer types */

#ifndef GL_INTPROPS_H
# define GL_INTPROPS_H

# include <limits.h>

/* True if negative values of the signed integer type T use two's
   complement, ones' complement, or signed magnitude representation,
   respectively.  Much GNU code assumes two's complement, but some
   people like to be portable to all possible C hosts */
# define TYPE_SIGNED_MAGNITUDE(t) ((t) ~ (t) < (t) -1)

/* True if the arithmetic type T is signed */
# define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The maximum and minimum values for the integer type T. These
   macros have undefined behavior if T is signed and has padding bits.
   If this is a problem for you, please let us know how to fix it for
   your host. */

# define TYPE_MINIMUM(t) \
    ((t) (! TYPE_SIGNED (t) \
            ? (t) 0 \
            : TYPE_SIGNED_MAGNITUDE (t) \
                ? ~ (t) 0 \
                : ! (t) 0 << (sizeof (t) * CHAR_BIT - 1)))

# define TYPE_MAXIMUM(t) \
    ((t) (! TYPE_SIGNED (t) \
            ? (t) -1 \
            : ~ (~ (t) 0 << (sizeof (t) * CHAR_BIT - 1))))

#endif
