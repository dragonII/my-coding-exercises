/* utmp_.h      interfaces for self wrappers in utmp.c */

#ifndef UTMP_HEADER__
#define UTMP_HEADER__

/* Log an event in the wtmp file (reboot, runlevel) */
void write_wtmp(char *user,     /* name of user */
                char *id,       /* inittab ID */
                int  pid,       /* PID of process */
                int  type,      /* TYPE of entry */
                char *line);    /* which line is this */

#endif
