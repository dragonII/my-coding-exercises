#ifndef GROUP_LIST_H
#define GROUP_LIST_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

bool print_group_list(const char* username,
                      uid_t ruid, gid_t rgid, gid_t egid,
                      bool use_names);

bool print_group(gid_t gid, bool use_names);

#endif
