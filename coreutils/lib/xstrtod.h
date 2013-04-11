/* Error-checking interface to strtod-like functions */

#ifndef XSTRTOD_H
#define XSTRTOD_H

#include <stdbool.h>

bool xstrtod(char* str, const char** ptr, double* result,
                double (*convert)(char*, char**));
bool xstrtold(char* str, const char** ptr, double* result,
                double (*convert)(char*, char**));

#endif
