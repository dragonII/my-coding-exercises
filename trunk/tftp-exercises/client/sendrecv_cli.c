/*
 * Send and receive packets.
 */

#include "defs_cli.h"
#include "error_cli.h"
#include "netudp_cli.h"
#include "file_cli.h"

#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*
 * Send a RRQ or WRQ to the other system.
 * These two packets are only sent by the client to the server.
 * This function is called when either the "get" command or the
 * "put" command is executed by the user.
 */

void send_RQ(int opcode, char* fname, int mode)
{
    int   len;
    char* modestr;

    D_printf("send_RQ: sending RRQ/WRQ for %s, mode = %d\n", fname, mode);

    stshort(opcode, sendbuff);

    strcpy(sendbuff + 2, fname);
    len = 2 + strlen(fname) + 1;        // +1 for null byte at the end of fname

    switch(mode)
    {
        case MODE_ASCII:    modestr = "netascii"; break;
        case MODE_BINARY:   modestr = "octet";    break;
        default:
            err_quit("send_RQ: unknown mode");
    }

    strcpy(sendbuff + len, modestr);
    len += strlen(modestr) + 1;         // +1 for null byte at the end of modestr

    sendlen = len;
    net_send(sendbuff, sendlen);

    opcode = opcode;
}


/*
 * Error packet received in response to an RRQ or a WRQ.
 * Usually means the file we're asking for on the other system
 * can't be accessed for the reason. We need to print the 
 * error message that's returned.
 * Called by finite state machine.
 */

int recv_RQERR(char* ptr, int nbytes)
{
    int ecode;

    ecode = ldshort(ptr);
    ptr += 2;
    nbytes -= 2;
    ptr[nbytes] = 0;    // assume it's null terminated

    D_printf("ERROR received, %d bytes, error code %d\n", nbytes, ecode);

    fflush(stdout);
    fprintf(stderr, "Error# %d: %s\n", ecode, ptr);
    fflush(stderr);

    return -1;          // terminate finite state loop
}


/*
 * Send an acknowledgement packet to the other system.
 * Called by the recv_DATA() function and also called
 * by recv_WRQ().
 */

void send_ACK(int blocknum)
{
    D_printf("send_ACK: sending ACK for block# %d\n", blocknum);

    stshort(OP_ACK, sendbuff);
    stshort(blocknum, sendbuff + 2);

    sendlen = 4;
    net_send(sendbuff, sendlen);

#ifdef SORCERER
    if(blocknum == 1)
        net_send(sendbuff, sendlen);        // send the first ACK twice
#endif

    op_sent = OP_ACK;
}

/*
 * Send data to the other system.
 * The data must be stored in the "sendbuf" by the caller.
 * Called by the recv_ACK() function.
 */

void send_DATA(int blocknum, int nbytes)
{
    D_printf("send_DATA: sending %d bytes of DATA with block# %d\n",
                    nbytes, blocknum);

    stshort(OP_DATA, sendbuff);
    stshort(blocknum, sendbuff + 2);

    sendlen = nbytes + 4;
    net_send(sendbuff, sendlen);
    op_sent = OP_DATA;
}


/*
 * Data packet received. Send a acknowledgement.
 * Called by finite state machine.
 */

int recv_DATA(char* ptr, int nbytes)
{
    int recvblknum;

    recvblknum = ldshort(ptr);
    ptr += 2;
    nbytes -= 2;

    D_printf("recv_DATA: data received %d bytes, block# %d\n",
                    nbytes, recvblknum);

    if(recvblknum == nextblknum)
    {
        /*
         * The data packet is the expected one.
         * Increment our expected-block# for the next packet.
         */

        nextblknum++;
        totnbytes += nbytes;

        if(nbytes > 0)
        {
            /*
             * Note that the final data packet can have a
             * data length of zero, so we only write the
             * data to the local file if there is data.
             */

            file_write(localfp, ptr, nbytes, modetype);
        }
    } else if(recvblknum < (nextblknum - 1))
    {
        err_quit("recv_DATA: recvblknum < nextblknum - 1");
        exit(-1);
    } else if(recvblknum > nextblknum)
    {
        err_quit("recv_DATA: recvblknum > nextblknum");
        exit(-1);
    }

    /*
     * The only case not handled above is "recvblknum == (nextblknum - 1)"
     * This means the other end never saw our ACK for the last data
     * packet and retransmitted it. We just ignore the retransmission
     * and send another ACK.
     *
     * Acknowledge the data packet.
     */

    send_ACK(recvblknum);

    /*
     * If the length of data is between 0-511, we've just
     * received the final data packet, else there is more to come.
     * -1 to terminate the finite state machine.
     */

    return ( (nbytes == MAXDATA) ? 0 : -1);
}


/*
 * ACK packet received. Send some more data.
 * Called by finite state machine. 
 */

int recv_ACK(char* ptr, int nbytes)
{
    int recvblknum;

    recvblknum = ldshort(ptr);
    if(nbytes != 2)
    {
        err_quit("recv_ACK: packet received with length = %d bytes",
                        nbytes + 2);
        exit(-1);
    }
    D_printf("recv_ACK: received, block# %d\n", recvblknum);

    if(recvblknum == nextblknum)
    {
        if((nbytes = file_read(localfp, sendbuff + 4, MAXDATA, modetype)) == 0)
        {
            if(lastsend < MAXDATA)
                return -1;      // done
            // else we'll send nbytes=0 of data
        }

        lastsend = nbytes;
        nextblknum++;        // incr for this new packet of data
        totnbytes += nbytes;
        send_DATA(nextblknum, nbytes);

        return 0;
    } else if(recvblknum < (nextblknum - 1))
    {
        err_quit("recv_ACK: recvblknum < nextblknum - 1");
        exit(-1);
    } else if(recvblknum > nextblknum)
    {
        err_quit("recv_ACK: recvblknum > nextblknum");
        exit(-1);
    } else
    {
        /*
         * Here we have "recvblknum == (nextblknum - 1)"
         * This means we received a duplicate ACK. This means either:
         *  (1) the other side never received our last data packet;
         *  (2) the other side's ACK got delayed somehow.
         *
         * If we were to retransmit the last data packet, we would start
         * the "Sorcerer's Apprentice Syndrome", we'll just ignore this
         * duplicate ACK, returning to the FSM loop, which will initiate
         * another receive.
         */

        return 0;
    }
}
