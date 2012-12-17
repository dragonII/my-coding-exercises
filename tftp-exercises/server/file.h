#ifndef __FILE_H__
#define __FILE_H__


FILE* file_open(char* filename, char* mode, int initblknum);
void  file_close(FILE* fp);
int   file_read(FILE* fp, char* ptr, int maxnbytes, int mode);
void  file_write(FILE* fp, char* ptr, int nbytes, int mode);

#endif
