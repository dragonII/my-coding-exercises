#ifndef __PTRX_PROCESS_CYCLE_H__
#define __PTRX_PROCESS_CYCLE_H__

pid_t           ptrx_pid;

int             ptrx_threads_n;

unsigned int    ptrx_inherited;
unsigned int    ptrx_process;
unsigned int    ptrx_daemonized;


#define PTRX_PROCESS_SINGLE      0
#define PTRX_PROCESS_MASTER      1
#define PTRX_PROCESS_SIGNALLER   2
#define PTRX_PROCESS_WORKER      3
#define PTRX_PROCESS_HELPER      4



#endif
