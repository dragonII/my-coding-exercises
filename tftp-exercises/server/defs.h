/*
 * Definitions for TFTP
 */

#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>

//#undef __DEBUG__   // used for closing debugging output

#ifdef __DEBUG__
#define D_printf(fmt, arg...) fprintf(stderr, fmt, ##arg)
#else
#define D_printf(fmt, arg...)
#endif

#define MAXBUFF     2048 // transmit and receive buffer length
#define MAXDATA     512  // max size of data per package to send or rcv
                         // 512 is specified by RFC
#define MAXFILENAME 128  // max filename length
#define MAXHOSTNAME 128  // max host name length
#define MAXLINE     512  // max command line length
#define MAXTOKEN    128  // max token length
#define MAXHOSTNAMELEN  64 // max size of a host name

#define TFTP_SERVICE "tftp" // name of the service
#define DAEMONLOG    "/tmp/tftpd.log" // log for daemon tracing

/*
 * Externals
 */

extern char command[];      // the command being processed
extern int  connected;      // true if we're connected to host
extern char hostname[];     // name of host system
extern int  inetdflag;      // true if we were started by a daemon
extern int  interactive;    // true if we're running interactive
extern jmp_buf jmp_mainloop;// to return to main command loop
extern int  lastsend;       // #bytes of data in last data package
extern FILE *localfp;       // fp of local file to read or write
extern int  modetype;       // see MODE_xxx values
extern int  nextblknum;     // next block# to send or rcv
extern int  *pname;         // the name by which we are invoked
extern int  port;           // port# - host by order, 0 -> use default
extern char *prompt;        // prompt string, for interactive use
extern long totnbytes;      // for get or put statistics printing
extern int  traceflag;      // -t command line option, or "trace" cmd
extern int  verboseflag;    // -v command line option

#define MODE_ASCII  0       // ascii == netascii
#define MODE_BINARY 1       // binary == octet

/*
 * One receive buffer and one transmit buffer
 */

extern char recvbuff[];
extern char sendbuff[];
extern int  sendlen;        // #bytes in sendbuff[]

/*
 * Define the tftp opcodes
 */

#define OP_RRQ      1   // Read Request
#define OP_WRQ      2   // Write Request
#define OP_DATA     3   // Data
#define OP_ACK      4   // Acknowledgement
#define OP_ERROR    5   // Error, see error codes below

#define OP_MIN      1   // minimum opcode value
#define OP_MAX      5   // maximum opcode value

extern int op_sent;     // last opcode sent
extern int op_recv;     // last opcode received

/*
 * Define the tftp error codes.
 * These are transmitted in an error packet (OP_ERROR) with an
 * optional netascii Error Message describing the error.
 */

#define ERR_UNDEF   0   // not defined, see error message
#define ERR_NOFILE  1   // File not found
#define ERR_ACCESS  2   // Access denied
#define ERR_NOSPACE 3   // Disk full or allocation exceeded
#define ERR_BADOP   4   // Illegal tftp operation
#define ERR_BADID   5   // Unknown TID (port#)
#define ERR_FILE    6   // File already exists
#define ERR_NOUSER  7   // No such user


/*
 * Define macros to load and store 2-byte integers, since these are
 * used in the TFTP headers for opcodes, block numbers and error 
 * numbers. These macros handle the conversion between host format
 * and network byte ordering.
 */

#define ldshort(addr)       ( ntohs (*( (u_short *)(addr) ) ) )
#define stshort(sval, addr) ( *( (u_short *)(addr) ) = htons(sval) )

//#ifdef  lint   // hush up lint
//#undef  ldshort
//#undef  stshort
//short   ldshort();
//#endif  

/*
 * Datatypes of functions that dont return an int
 */

char    *gettoken();
double  t_getrtime();   // our library routine to return elasped time
char    *sys_err_str(); // our library routine for system error message

#endif
