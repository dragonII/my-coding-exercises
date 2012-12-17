#ifndef __DEFS_H_CLIENT__
#define __DEFS_H_CLIENT__

#include <setjmp.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAXBUFF     2048    // transmit and receive buffer length
#define MAXDATA      512    // max size of data per packet to send or recv
#define MAXFILENAME  128    // max filename length
#define MAXHOSTNAME  128    // max host name length
#define MAXLINE      512    // max command line length
#define MAXTOKEN     128    // max token length

/*
 * Externals
 */

extern int  port;
extern char command[];      // the command being processed
extern int  connected;      // true if we're connected to host
extern char hostname[];     // name of host system
extern int  interactive;    // true if we're running interactive
extern jmp_buf jmp_mainloop;    // to return to main command loop
extern int  lastsend;       // #bytes of data in last data packet
extern FILE* localfp;       // fp of local file to read or write
extern int  modetype;       
extern char *pname;         // the name by which we are invoked
extern char *prompt;        // prompt string, for interactive use
extern int  traceflag;      // -t command line option
extern int  verboseflag;    // -v command line option
extern char temptoken[MAXTOKEN];
extern long totnbytes;
extern int  nextblknum;

#define TFTP_SERVICE    "tftp"  // name of the service

#define MODE_ASCII  0        
#define MODE_BINARY 1

/*
 * One receive buffer and one transmit buffer.
 */

extern char recvbuff[];
extern char sendbuff[];
extern int  sendlen;        // #bytes in sendbuff[]

/*
 * Define the tftp opcodes.
 */

#define OP_RRQ      1
#define OP_WRQ      2
#define OP_DATA     3
#define OP_ACK      4
#define OP_ERROR    5

#define OP_MIN      1   // minimum opcode value
#define OP_MAX      5   // maximum opcode value

extern int  op_sent;    // last opcode sent
extern int  op_recv;    // last opcode received

/*
 * Define the tftp error codes
 * These are transmitted in an error packet (OP_ERROR) with an
 * optional netascii Error Message describing the error
 */

#define ERR_UNDEF   0   
#define ERR_NOFILE  1
#define ERR_ACCESS  2
#define ERR_NOSPACE 3
#define ERR_BADOP   4   // Illegal tftp operation
#define ERR_BADID   5   // Unknown TID (port#)
#define ERR_FILE    6   // File already exists
#define ERR_NOUSER  7


#ifdef __DEBUG__
#define D_printf(fmt, arg...) fprintf(stderr, fmt, ##arg)
#else
#define D_printf(fmt, arg...)
#endif

/*
 * Define macros to load and store 2-bytes integers, since these are
 * used in the TFTP headers for opcodes, block numbers and error
 * numbers. These macros handle the conversion between host format
 * and network byte ordering.
 */

#define ldshort(addr)       ( ntohs (*( (u_short *)(addr) ) ) )
#define stshort(sval, addr) ( *( (u_short *)(addr) ) = htons(sval) )


#endif
