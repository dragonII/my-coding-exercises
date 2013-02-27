/* closexec.c - set or clear the close-on-exec descriptor flag */

#ifndef CLOSE_ON_EXEC_H
#define CLOSE_ON_EXEC_H

#include <stdbool.h>

/* Set the `FD_CLOEXEC' flag of DESC if VALUE is true,
   or clear the flag if VALUE is false.
   Return 0 on success, or -1 on error with `errno' set.

   Note that on MingW, this function does NOT protect DESC from being
   inherited into spawned children. Instead, either use dup_cloexec
   followed by closing the original DESC, or use interfaces such as
   open or pipe2 that accept flags like O_CLOEXEC to create DESC
   non-inheritable in the first place. */
int set_closexec_flag(int desc, bool value);

/* Duplicates a file handle FD, while marking the copy to be closed
   prior to exec or spawn. Returns -1 and sets errno if FD could not
   be duplicated. */
int dup_cloexec(int fd);




#endif
