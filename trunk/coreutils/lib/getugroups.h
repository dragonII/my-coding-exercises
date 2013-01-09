#ifndef GETUGROUPS_H
#define GETUGROUPS_H

#include <sys/types.h>

int getugroups(int maxcount, gid_t* grouplist, const char* username, gid_t gid);

#endif
