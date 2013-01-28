#ifndef MODECHANGE_H_
#define MODECHANGE_H_

#include <stdbool.h>
#include <sys/types.h>

/* The traditional octal values corresponding to each mode bit. */
#define SUID 04000
#define SGID 02000
#define SVTX 01000
#define RUSR 00400
#define WUSR 00200
#define XUSR 00100
#define RGRP 00040
#define WGRP 00020
#define XGRP 00010
#define ROTH 00004
#define WOTH 00002
#define XOTH 00001
#define ALLM 07777  /* all octal mode bits */

struct mode_change* mode_compile(char* mode_string);
mode_t mode_adjust(mode_t oldmode, bool dir, mode_t umask_value,
                    struct mode_change* changes, mode_t* pmode_bits);


#endif
