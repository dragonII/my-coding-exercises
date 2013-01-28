/* Invoke unistd-like functions, but avoid some glitches */

#ifndef _UNISTD_SAFER_H
#define _UNISTD_SAFER_H

int dup_safer(int);
int fd_safer(int);
int pipe_safer(int[2]);


#endif
