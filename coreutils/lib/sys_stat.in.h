#ifndef _SYS_STAT_IN_H
#define _SYS_STAT_IN_H

#include <sys/stat.h>

/* S_IXUGO is a common extension to POSIX */

#if ! S_IXUGO
# define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

#ifndef S_IRWXUGO
# define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)
#endif

#endif
