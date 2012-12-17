/*
 * Finite state machine routines.
 */

#include "defs_cli.h"
#include "rtt_cli.h"
#include "netudp_cli.h"
#include "error_cli.h"
#include "rtt_cli.h"
#include "sendrecv_cli.h"

#include <signal.h>
#include <unistd.h>

static struct rtt_struct rttinfo;   // used by the rtt_XXX() functions.
static int    rttfirst = 1;

int tout_flag;                      // set to 1 by SIGALRM handler


/*
 * Invalid state transition. Something is wrong.
 */

int fsm_invalid(char* ptr, int nbytes)
{
    err_quit("fsm_invalid: protocol botch, op_sent = %d, op_recv = %d",
                    op_sent, op_recv);
    return -1;
}

/* 
 * Error packet received and we weren't expecting it.
 */

int fsm_error(char* ptr, int nbytes)
{
    err_quit("fsm_error: error received, op_sent = %d, op_recv = %d",
                    op_sent, op_recv);
    return -1;
}


/*
 * Signal handler for timeouts.
 * Just set the flag when the net_recv()
 * returns an error (interrupted system call).
 */

void func_timeout()
{
    tout_flag = 1;    
}


/*
 * Finite state machine table.
 * This is just a 2-d array indexed by the last opcode sent and
 * the opcode just received. The result is the address of a function
 * to call to process the received opcode.
 */

int (*fsm_ptr [ OP_MAX + 1] [ OP_MAX + 1] ) () = 
{
    {
        fsm_invalid,        // [sent = 0]       [recv = 0]
        fsm_invalid,        // [sent = 0]       [recv = OP_RRQ]
        fsm_invalid,        // [sent = 0]       [recv = OP_WRQ]
        fsm_invalid,        // [sent = 0]       [recv = OP_DATA]
        fsm_invalid,        // [sent = 0]       [recv = OP_ACK]
        fsm_invalid,        // [sent = 0]       [recv = OP_ERROR]
    },
    {
        fsm_invalid,        // [sent = OP_RRQ]  [recv = 0]
        fsm_invalid,        // [sent = OP_RRQ]  [recv = OP_RRQ]
        fsm_invalid,        // [sent = OP_RRQ]  [recv = OP_WRQ]
        recv_DATA,          // [sent = OP_RRQ]  [recv = OP_DATA]
        fsm_invalid,        // [sent = OP_RRQ]  [recv = OP_ACK]
        recv_RQERR,         // [sent = OP_RRQ]  [recv = OP_ERROR]
    },
    {
        fsm_invalid,        // [sent = OP_WRQ]  [recv = 0]
        fsm_invalid,        // [sent = OP_WRQ]  [recv = OP_RRQ]
        fsm_invalid,        // [sent = OP_WRQ]  [recv = OP_WRQ]
        fsm_invalid,        // [sent = OP_WRQ]  [recv = OP_DATA]
        recv_ACK,           // [sent = OP_WRQ]  [recv = OP_ACK]
        recv_RQERR,         // [sent = OP_WRQ]  [recv = OP_ERROR]
    },
    {
        fsm_invalid,        // [sent = OP_DATA]  [recv = 0]
        fsm_invalid,        // [sent = OP_DATA]  [recv = OP_RRQ]
        fsm_invalid,        // [sent = OP_DATA]  [recv = OP_WRQ]
        fsm_invalid,        // [sent = OP_DATA]  [recv = OP_DATA]
        recv_ACK,           // [sent = OP_DATA]  [recv = OP_ACK]
        fsm_error,          // [sent = OP_DATA]  [recv = OP_ERROR]
    },
    {
        fsm_invalid,        // [sent = OP_ACK]  [recv = 0]
        fsm_invalid,        // [sent = OP_ACK]  [recv = OP_RRQ]
        fsm_invalid,        // [sent = OP_ACK]  [recv = OP_WRQ]
        recv_DATA,          // [sent = OP_ACK]  [recv = OP_DATA]
        fsm_invalid,        // [sent = OP_ACK]  [recv = OP_ACK]
        fsm_error,          // [sent = OP_ACK]  [recv = OP_ERROR]
    },
    {
        fsm_invalid,        // [sent = OP_ERROR]  [recv = 0]
        fsm_invalid,        // [sent = OP_ERROR]  [recv = OP_RRQ]
        fsm_invalid,        // [sent = OP_ERROR]  [recv = OP_WRQ]
        fsm_invalid,        // [sent = OP_ERROR]  [recv = OP_DATA]
        fsm_invalid,        // [sent = OP_ERROR]  [recv = OP_ACK]
        fsm_error,          // [sent = OP_ERROR]  [recv = OP_ERROR]
    }
};


/*
 * Main loop of finite state machine.
 * 
 * We're called after either an RRQ or a WRQ has been sent
 * to the other side.
 *
 * Return 0 on normal termination, -1 on timeout.
 *      opcode: RRQ or WRQ
 */

int fsm_loop(int opcode)
{
    int nbytes;

    op_sent = opcode;

    if(rttfirst)
    {
        rtt_init(&rttinfo);
        rttfirst = 0;
    }

    rtt_newpack(&rttinfo);      // initilize for a new packet

    for( ; ; )
    {
        signal(SIGALRM, func_timeout);
        tout_flag = 0;
        alarm(rtt_start(&rttinfo));     // calc timeout & start timer

        if((nbytes = net_recv(recvbuff, MAXBUFF)) < 0)
        {
            if(tout_flag)
            {
                /*
                 * The receive timed out. See if we've tried
                 * enough, and if so, return to caller.
                 */

                if(rtt_timeout(&rttinfo) < 0)
                {
                    printf("fsm_loop: transfer timed out\n");
                    return -1;
                }
            } else
            {
                err_quit("fsm_loop: net_recv error");
            }

            /*
             * Retransmit the last packet.
             */

            net_send(sendbuff, sendlen);
            continue;
        }

        alarm(0);       // stop signal timer.
        tout_flag = 0;
        rtt_stop(&rttinfo);     // stop RTT timer, calc new values

        if(nbytes < 4)
            err_quit("fsm_loop: receive length = %d bytes", nbytes);

        op_recv = ldshort(recvbuff);

        if(op_recv < OP_MIN || op_recv > OP_MAX)
            err_quit("fsm_loop: invalid opcode received %d", op_recv);

        /*
         * We call the appropriate function, passing the address
         * of the receive buffer and its length. These arguments
         * ignore the received-opcode, which we've already processed.
         *
         * We assume the called function will send a response to the
         * other side. It is the called function's responsibility to
         * set op_sent to the op-code that it sends to the other side.
         */

        if((*fsm_ptr[op_sent][op_recv])(recvbuff + 2, nbytes - 2) < 0)
        {
            /*
             * When the called function returns -1, this loop
             * is done. Turn off the signal handler for
             * timeouts and return to the caller.
             */

            signal(SIGALRM, SIG_DFL);
            return 0;
        }
    }
}



