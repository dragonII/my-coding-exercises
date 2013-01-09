#ifndef MGETGROUPS_H
#define MGETGROUPS_H

#include <sys/types.h>

int mgetgroups(const char* username, gid_t gid, gid_t** groups);
int xgetgroups(const char* username, gid_t gid, gid_t** groups);

#endif
