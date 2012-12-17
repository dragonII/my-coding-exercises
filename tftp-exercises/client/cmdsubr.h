#ifndef __CMDSUBR_H__
#define __CMDSUBR_H__

#include <stdio.h>

int get_line(FILE* fp);
char* gettoken(char* token);
void striphost(char* fname, char* hname);
void docmd(char* cmdptr);

#endif
