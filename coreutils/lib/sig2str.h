/* sig2str.h -- convert between signal names and numbers */

#ifndef SIG2STR_HEADER
#define SIG2STR_HEADER

#include <signal.h>

/* Don't override system declarations of SIG2STR_MAX, sig2str, str2sig */
#ifndef SIG2STR_MAX
# include "intprops.h"

/* Size of a buffer needed to hold a signal name like "HUP" */
# define SIG2STR_MAX (sizeof "SIGRTMAX" + INT_STRLEN_BOUND(int) - 1)

# define SIGNUM_BOUND 64

int sig2str(int, char*);
int str2sig(char*, int*);

#endif

#endif
