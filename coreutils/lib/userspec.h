#ifndef USERSPEC_H
#define USERSPEC_H

#include <sys/types.h>

char* parse_user_spec(char* spec, uid_t* uid, gid_t* gid,
                        char** username, char** groupname);

#endif
