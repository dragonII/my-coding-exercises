/* Invoke stdio functions, but avoid some glitches */

#ifndef STDIO_SAFER_HEADER
#define STDIO_SAFER_HEADER

#include <stdio.h>

FILE* fopen_safer(char*, char*);


#endif
