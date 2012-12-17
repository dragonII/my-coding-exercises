/*
 * Initialize the external variables
 */

#include "defs.h"

char command[MAXTOKEN]  = {0};
int  connected          = 0;
char hostname[MAXHOSTNAME]  = {0};
int  inetdflag              = 1;
int  interactive            = 1;
int  lastsend               = 0;
FILE *localfp               = NULL;
int  modetype               = MODE_ASCII;
int  nextblknum             = 0;
int  port                   = 0;
char *prompt                = "tftp: ";
char recvbuff[MAXBUFF]      = {0};
char sendbuff[MAXBUFF]      = {0};
int  sendlen                = 0;
long totnbytes              = 0;
int  op_sent                = 0;
int  op_recv                = 0;

jmp_buf jmp_mainloop;
