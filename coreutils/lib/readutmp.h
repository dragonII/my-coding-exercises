/* Declarations for GNU's read utmp module */

#ifndef __READUTMP_H
#define __READUTMP_H

#include <utmp.h>

#define UTMP_STRUCT_NAME utmp
#define SET_UTMP_ENT setutent
#define GET_UTMP_ENT getutent
#define END_UTMP_ENT endutent
#define UTMP_NAME_FUNCTION utmpname

typedef struct UTMP_STRUCT_NAME STRUCT_UTMP;

int read_utmp(const char* file, size_t *n_entries, STRUCT_UTMP **utmp_buf,
                int options);


#endif
