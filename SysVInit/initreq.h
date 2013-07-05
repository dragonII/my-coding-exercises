/*
 * initreq.h        Interface to talk to init through /dev/initctl 
 */
#ifndef __INITREQ_HEADER__
#define __INITREQ_HEADER__

#define INIT_FIFO           "/dev/initctl"

#define INIT_MAGIC              0x03091969
#define INIT_CMD_START          0
#define INIT_CMD_RUNLVL         1
#define INIT_CMD_POWERFAIL      2
#define INIT_CMD_POWERFAILNOW   3
#define INIT_CMD_POWEROK        4
#define INIT_CMD_BSD            5
#define INIT_CMD_SETENV         6
#define INIT_CMD_UNSETENV       7

#define INIT_CMD_CHANGECONS     12345


#ifdef MAXHOSTNAMELEN
# define INITRQ_HLEN    MAXHOSTNAMELEN
#else
# define INITRQ_HLEN    64
#endif

/*
 * This is what BSD 4.4 uses when talking to init.
 * Linux doesn't use this right now.
 */
struct init_request_bsd
{
    char    gen_id[8];  
    char    tty_id[16];         /* tty name minus uses "fe" */
    char    host[INITRQ_HLEN];  /* hostname */
    char    term_type[16];      /* terminal type */
    int     signal;             /* signal to send */
    int     pid;                /* process to send to */
    char    exec_name[128];     /* program to execute */
    char    reserved[128];      /* for future expansion */
};

/* Because of legacy interfaces, "runlevel" and "sleeptime"
   aren't in a separate struct in the union.
   
   The weird sizes are because init expects the whole
   struct to be 384 bytes.
 */
struct init_request
{
    int magic;      /* magic number */
    int cmd;        /* what kind of request */
    int runlevel;   /* runlevel to change to */
    int sleeptime;  /* time between TERM and KILL */
    union
    {
        struct init_request_bsd bsd;
        char    data[368];
    } i;
};


#endif
