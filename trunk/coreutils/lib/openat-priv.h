/* Internals for openat-like functions */

#ifndef _GL_HEADER_OPENAT_PRIV
#define _GL_HEADER_OPENAT_PRIV

#include <errno.h>
#include <stdlib.h>

#ifndef OPENAT_BUFFER_SIZE
#define OPENAT_BUFFER_SIZE 512
#endif

char* openat_proc_name(char buf[OPENAT_BUFFER_SIZE], int fd, char* file);

#endif
