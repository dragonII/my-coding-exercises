/* inttostr.h -- convert integers to printable strings */

#ifndef INTTOSTR_H
#define INTTOSTR_H

#include <stdint.h>
#include <sys/types.h>

#include "intprops.h"

#define inttostr umaxtostr
#define inttype uintmax_t
#define inttype_is_signed 0

char* offtostr(off_t, char*);
char* imaxtostr(intmax_t, char*);
char* umaxtostr(uintmax_t, char*);
char* uinttostr(unsigned int, char*);


#endif
