#ifndef EUIDACCESS_STAT_HEADER
#define EUIDACCESS_STAT_HEADER

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

bool euidaccess_stat(struct stat* st, int mode);


#endif
