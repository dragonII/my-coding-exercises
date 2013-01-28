#ifndef _DIRCHOWNMODE_H
#define _DIRCHOWNMODE_H

#include <sys/types.h>

int dirchownmod(int, char*, mode_t, uid_t, gid_t, mode_t, mode_t);


#endif
