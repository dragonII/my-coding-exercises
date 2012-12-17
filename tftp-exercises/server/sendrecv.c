/*
 * Send and receive packets.
 */

#include "defs.h"
#include "net_udp.h"
#include "file.h"

#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*
 * Copy a string and convert it to lower case in the process.
 */
void strlccpy(char* dest, char* src)
{
    char c;
    while((c = *src++) != '\0')
    {
        if(isupper(c))
            c = tolower(c);
        *dest++ = c;
    }
    *dest = 0;
}


/*
 * Send an error packet.
 * Note that an error packet isn't retransmitted or acknowledged by
 * the other end, so once we're done sending it, we can exit.
 */
void send_ERROR(int ecode, char* errstring)
{
    D_printf("send_ERROR: sending ERROR, code = %d, string = %s\n", ecode, errstring);

    stshort(OP_ERROR, sendbuff);
    stshort(ecode, sendbuff + 2);

    strcpy(sendbuff + 4, errstring);

    sendlen = 4 + strlen(sendbuff + 4) + 1;     // +1 for null at end
    net_send(sendbuff, sendlen);

    net_close();

    exit(0);
}


/*
 * Process an RRQ or WRQ that has been received.
 * Called by the 2 routines above.
 */
void recv_xRQ(int opcode, char* ptr, int nbytes)
{
    int  i;
    char *saveptr;
    char filename[MAXFILENAME], dirname[MAXFILENAME], mode[MAXFILENAME];
    struct stat statbuff;

    /*
     * Assure the filename and mode are present and
     * null-terminated.
     */

    saveptr = ptr;      // points to beginning of filename
    for(i = 0; i < nbytes; i++)
        if(*ptr++ == '\0')
            goto FileOK;
    {
        D_printf("recv_xRQ: Invalid filename\n");
        exit(-1);
    }

FileOK:
    strcpy(filename, saveptr);
    saveptr = ptr;      // points to beginning of Mode

    for(; i < nbytes; i++)
        if(*ptr++ == '\0')
            goto ModeOK;
    {
        D_printf("recv_xRQ: Invalid Mode\n");
        exit(-1);
    }

ModeOK:
    strlccpy(mode, saveptr);    // copy and convert to lower case

    if(strcmp(mode, "netascii") == 0)
        modetype = MODE_ASCII;
    else if(strcmp(mode, "octet") == 0)
        modetype = MODE_BINARY;
    else
        send_ERROR(ERR_BADOP, "Mode isn't netascii or octet");

    /*
     * Validate the filename.
     * Note that as a daemon we might be running with root
     * privileges. Since there are no user-access checks with
     * tftp (as compared to ftp, for example) we will only
     * allow access to files that are publicly accessable.
     *
     * Also, since we're running as a daemon, our home directory
     * is the root, so any filename must have it's full
     * pathname specified
     */

    if(filename[0] != '/')
        send_ERROR(ERR_ACCESS, "filename must begin with '/'");

    if(opcode == OP_RRQ)
    {
        /*
         * Read request - verify that the file exists
         * and that it has world read permission.
         */

        if(stat(filename, &statbuff) < 0)
            send_ERROR(ERR_ACCESS, "filename get stat buffer error");
        if((statbuff.st_mode & (S_IREAD >> 6)) == 0)  // S_IROTH
            send_ERROR(ERR_ACCESS, "File doesn't allow world read permission");
    } else if(opcode == OP_WRQ)
    {
        /*
         * Write request - verify that the directoy
         * that the file is being written to has world
         * write permission. We've already verified above
         * that the filename starts with a '/'.
         */

        char *rindex();

        strcpy(dirname, filename);
        *(rindex(dirname, '/') + 1) = '\0';
        if(stat(dirname, &statbuff) < 0)
            send_ERROR(ERR_ACCESS, "dirname get stat buffer error");
        if((statbuff.st_mode & (S_IWRITE >> 6)) == 0)  // S_IWOTH
            send_ERROR(ERR_ACCESS, "Directory doesn't allow world write permission");

     } else
     {
         D_printf("recv_xRQ: unknown opcode\n");
         exit(-1);
     }

     localfp = file_open(filename, (opcode == OP_RRQ) ? "r" : "w", 0);
     if(localfp == NULL)
         send_ERROR(ERR_NOFILE, "file open error");
}




/*
 * Send an acknowledgement packet to the other system.
 * Called by the recv_DATA() function below and also called by
 * recv_WRQ()
 */
void send_ACK(int blocknum)
{
    D_printf("send_ACK: sending ACK for block# %d\n", blocknum);

    stshort(OP_ACK, sendbuff);
    stshort(blocknum, sendbuff + 2);

    sendlen = 4;
    net_send(sendbuff, sendlen);

#ifdef SORCERER
    /*
     * If you want to see the Sorcerer's Apprentice syncdrome,
     * #define SORCERER
     */
    if(blocknum == 1)
        net_send(sendbuff, sendlen); // send the first packet twice
#endif
    
    op_sent = OP_ACK;
}


/*
 * Send data to the other system.
 * The data must be stored in the "sendbuff" by the caller.
 * Called by the recv_ACK() function below.
 */
void send_DATA(int blocknum, int nbytes)
{
    D_printf("send_DATA: sending %d bytes of DATA with blocknum# %d\n", nbytes, blocknum);

    stshort(OP_DATA, sendbuff);
    stshort(blocknum, sendbuff + 2);
    sendlen = nbytes + 4;
    net_send(sendbuff, sendlen);
    op_sent = OP_DATA;
}

/*
 * DATA packet recevied. Send a acknowledgement.
 * Called by finite state machine.
 */
int recv_DATA(char* ptr, int nbytes)
{
    int recvblknum;

    recvblknum = ldshort(ptr);
    ptr += 2;
    nbytes -= 2;

    D_printf("recv_DATA: data received %d bytes, block# %d\n", nbytes, recvblknum);

    if(nbytes > MAXDATA)
    {
        D_printf("recv_DATA: data packet received with length = %d bytes\n", nbytes);
        exit(-1);
    }

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

        /*
         * If the length of the data is between 0-511, this is
         * the last data block. Here's where we have to close
         * the file. 
         */

         if(nbytes < MAXDATA)
             file_close(localfp);
    } else if(recvblknum < (nextblknum - 1))
    {
        /*
         * We've just received data block# N (or earlier, such as N-1,
         * N-2, etc.) from the other end, but we were expecting data
         * block# N+2. But if we were expecting N+2 it means we've
         * already received N+1, so the other end went backwards from
         * N+1 to N (or earlier). Something is wrong.
         */

        D_printf("recv_DATA: recvblknum < nextblknum - 1\n");
        exit(-1);
    } else if(recvblknum > nextblknum)
    {
        /*
         * We've just received data block# N (or later, such as N+1,
         * N+2, etc.) from the other end, but we were expecting data
         * block# N-1. But this implies that the other end has
         * received an ACK for block# N-1 from us. Something is wrong.
         */

        D_printf("recv_DATA: recvblknum > nextblknum\n");
        exit(-1);
    }

    /* 
     * The only case not handled above is "recvblknum == (nextblknum - 1)".
     * This means the other end never saw our ACK for the last data
     * packet and retransmitted it. We just ignore the retransmission
     * and send another ACK.
     *
     * Acknowledge the data packet.
     */

    send_ACK(recvblknum);

    /*
     * If the length of the data is between 0-511, we've just
     * received the final data packet, else there is more to come.
     */

    return ((nbytes == MAXDATA) ? 0 : -1);
}

/*
 * ACK packet received. Send some more data.
 * Called by finite state machine. Also called by recv_RRQ() to
 * start the transmission of a file to the client.
 */

int recv_ACK(char* ptr, int nbytes)
{
    int recvblknum;

    recvblknum = ldshort(ptr);
    if(nbytes != 2)
    {
        D_printf("recv_ACK: ACK packet received with length = %d bytes\n", nbytes + 2);
        exit(-1);
    }

    D_printf("recv_ACK: ACK received, block# %d\n", recvblknum);

    if(recvblknum == nextblknum)
    {
        /*
         * The received acknowledgement is for the expected data
         * packet that we sent.
         * Fill the transmit buffer with the next block of data
         * to send.
         * If there's no more data to send, then we might be
         * finished. Note that we must send a final data packet
         * containing 0-511 bytes of data. If the length of the
         * last packet that we sent was exactly 512 bytes, then we
         * must send a 0-length data packet.
         */

        if((nbytes = file_read(localfp, sendbuff + 4, MAXDATA, modetype)) == 0)
        {
            if(lastsend < MAXDATA)
                return -1;  // done
            // else we'll send nbytes=0 of data
        }

        lastsend = nbytes;
        nextblknum++;
        totnbytes += nbytes;
        send_DATA(nextblknum, nbytes);

        return 0;
    } else if(recvblknum < (nextblknum - 1))
    {
        /*
         * We've just received the ACK for block# N (or earlier, such
         * as N-1, N-2, etc.) from the other end, but we were expecting
         * the ACK for block# N+2. But if we're expecting the ACK for
         * N+2 it means we've already received the ACK for N+1, so the
         * other end went backwards from N+1 to N (or earlier).
         * Something is wrong.
         */

        D_printf("recv_ACK: recvblknum < nextblknum - 1\n");
        exit(-1);
    } else if(recvblknum > nextblknum)
    {
        /*
         * We've just received the ACK for block# N (or earlier, such
         * as N+1, N+2, etc.) from the other end, but we were expecting
         * the ACK for block# N-1. But this implies that the other
         * end has already received data block# N-1 from us.
         * Something is wrong.
         */

        D_printf("recv_ACK: recvblknum > nextblknum\n");
        exit(-1);
    } else
    {
        /* 
         * Here we have "recvblknum == (nextblknum - 1)".
         * This means we received a duplicate ACK. This measn either:
         * (1) the other side never received our last data packet;
         * (2) the other side's ACK got delayed somehow.
         *
         * If we were to retransmit the last data packet, we would start
         * the "Sorcerer's Apprentice Syndrome." We'll just ignore this
         * duplicate ACK, returning to the FSM loop, which will initiate
         * another receive.
         */

        return 0;
    }
}

/*
 * RRQ packet received.
 * Called by the finite state machine.
 * This (and receiving a WRQ) are the only ways the server gets started.
 */
int recv_RRQ(char* ptr, int nbytes)
{
    char ackbuff[2];

    recv_xRQ(OP_RRQ, ptr, nbytes);      // verify the RRQ packet

    /*
     * Set things up so we can just call recv_ACK() and pretend we
     * received an ACK, so it'll send the first data block to the
     * client.
     */

    lastsend = MAXDATA;
    stshort(0, ackbuff);        // pretend its an ACK of block# 0

    recv_ACK(ackbuff, 2);       // this sends data block# 1

    return 0;                   // the finite state machine takes over from here
}

/*
 * WRQ packet received.
 * Called by the finite state machine.
 * This (and receiving an RRQ) are the only ways the server get started.
 */
int recv_WRQ(char* ptr, int nbytes)
{
    recv_xRQ(OP_WRQ, ptr, nbytes);      // verify the WRQ packet

    /*
     * Call send_ACK() to acknowledge block# 0, which will cause
     * the client to send data block# 1
     */

    nextblknum = 1;
    send_ACK(0);

    return 0;           // the finite state machine takes over from here
}


