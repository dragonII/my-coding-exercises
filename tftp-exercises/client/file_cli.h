#ifndef __FILE_OP_H__
#define __FILE_OP_H__

#include <stdio.h>

FILE* file_open(char* fname, char* mode, int initblknum);
void file_close(FILE* fp);
int file_read(FILE* fp, char* ptr, int maxnbytes, int mode);
void file_write(FILE* fp, char* ptr, int nbytes, int mode);

#endif
