/* Invode fcntl-like functions, but avoid some glitches */

#ifndef _FCNTL_SAFER_H
#define _FCNTL_SAFER_H

#include <sys/types.h>

int open_safer(char*, int, ...);
int create_safer(char*, mode_t);


#endif
