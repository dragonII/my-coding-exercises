/* An interface to read and write that retries after interrupts */

#ifndef SAFE_READ_H
#define SAFE_READ_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAFE_READ_ERROR ((size_t) -1)

/* Read up to COUNT bytes at BUF from descriptor FD, retying if interrupted.
   Return the actual number of bytes read, zero for EOF, or SAFE_READ_ERROR
   upon error. */
size_t safe_read(int fd, void* buf, size_t count);


#ifdef __cplusplus
}
#endif



#endif
