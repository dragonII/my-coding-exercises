#ifndef __PTRX_PROCESS_H__
#define __PTRX_PROCESS_H__

#define MAX_PROCESSES       1024

#define PTRX_PROCESS_NORESPAWN       -1
#define PTRX_PROCESS_JUST_SPAWN      -2
#define PTRX_PROCESS_RESPAWN         -3
#define PTRX_PROCESS_JUST_RESPAWN    -4
#define PTRX_PROCESS_DETACHED        -5

#define ptrx_getpid     getpid

int       ptrx_argc;
char    **ptrx_argv;
char    **ptrx_os_argv;

int ptrx_init_signals(ptrx_log_t *log);

#endif
