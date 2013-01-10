/* Declarations for GNU's read utmp module */

#ifndef __READUTMP_H
#define __READUTMP_H

#include <utmp.h>

#define UT_TYPE_NOT_DEFINED 0

#define UTMP_STRUCT_NAME utmp
#define SET_UTMP_ENT setutent
#define GET_UTMP_ENT getutent
#define END_UTMP_ENT endutent
#define UTMP_NAME_FUNCTION utmpname

#define UT_TIME_MEMBER(UT_PTR) ((UT_PTR)->ut_time)

#define UT_TYPE_EQ(U, V) ((U)->ut_type == (V))

#define UT_USER(Utmp) ((Utmp)->ut_user)

#define UT_PID(U) ((U)->ut_pid)

#define UT_TYPE_USER_PROCESS(U) UT_TYPE_EQ(U, USER_PROCESS)


#define IS_USER_PROCESS(U)                                      \
    (UT_USER (U)[0]                                             \
     && (UT_TYPE_USER_PROCESS (U)                               \
        || (UT_TYPE_NOT_DEFINED && UT_TIME_MEMBER (U) != 0)))


/* Options for read_utmp */
enum
{
    READ_UTMP_CHECK_PIDS = 1,
    READ_UTMP_USER_PROCESS = 2
};

typedef struct UTMP_STRUCT_NAME STRUCT_UTMP;

int read_utmp(const char* file, size_t *n_entries, STRUCT_UTMP **utmp_buf,
                int options);

bool desirable_utmp_entry(STRUCT_UTMP* u, int options);

char* extract_trimmed_name(const STRUCT_UTMP* ut);

#endif
