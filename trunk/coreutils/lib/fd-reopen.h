/* Invoke open, but return either a desired file descriptor or -1 */

#ifndef FD_REOPEN_H
#define FD_REOPEN_H

#include <sys/types.h>

int fd_reopen(int, char*, int, mode_t);


#endif
