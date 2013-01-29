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

/* Return zero if T can be determined to be an unsigned type.
   Otherwise, return 1.
   When compiling with GCC, INT_STRLEN_BOUND uses this macro to obtain a
   tighter bound. Otherwise, it overestimates the true bound by one byte
   when applied to unsigned types of size 2, 4, 16, ... bytes.
   The symbol signed_type_expr__ is private to this header file */
# if __GNUC__ >= 2
#  define signed_type_or_expr__(t) TYPE_SIGNED (__typeof__ (t))
# else
#  define signed_type_or_expr__(t) 1
# endif


/* Bound on length of the string representing an integer type or expression T.
   Subtract 1 for the sign bit if T is signed; log10 (2.0) < 146/485;
   add 1 for integer division truncation; add 1 more for minus sign
   if needed */
# define INT_STRLEN_BOUND(t) \
       ((sizeof (t) * CHAR_BIT - signed_type_or_expr__ (t)) * 146 / 485 \
       + signed_type_or_expr__ (t) + 1)

/* Bound on buffer size needed to represent an integer type or expression T,
   including the terminating null. */
# define INT_BUFSIZE_BOUND(t) (INT_STRLEN_BOUND (t) + 1)

#endif
