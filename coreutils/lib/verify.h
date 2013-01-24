/* Compile-time assert-like macros */

#ifndef VERIFY_H
#define VERIFY_H

/* Verify requirement R at compile-time, as an integer constant expression.
   Return 1. */

#define verify_true(R) \
    (!!sizeof \
        (struct { unsigned int verify_error_if_negative_size__: (R) ? 1 : -1; } ))



#endif
