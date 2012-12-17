/*
 * Finite state machine routines.
 */

#include "defs.h"
#include "rtt.h"
#include "net_udp.h"
#include "sendrecv.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

static struct rtt_struct  rttinfo;  // used by rtt_XXX() functions
static int    rttfirst = 1;

int tout_flag;  // set to 1 by SIGALRM handler


/*
 * Error packet received and we weren't expecting it.
 */

int fsm_error(char* ptr, int nbytes)  
{
    D_printf("error received: op_sent = %d, op_recv = %d\n", op_sent, op_recv);
    exit(-1);
}

/*
 * Invalid state transition. Something is wrong.
 */

int fsm_invalid(char* ptr, int nbytes)
{
    D_printf("protocol botch: op_sent = %d, op_recv = %d\n",
                op_sent, op_recv);
    exit(-1);
}

/*
 * Timeout signal handler
 */

void func_timeout()
{
    tout_flag = 1;
}


/*
 * Finite state machine table.
 * This is just a 2-d array indexed by last opcode sent and
 * the opcode just received. The result is the address of a 
 * function to call to process the received opcode.
 */

int (*fsm_ptr [OP_MAX + 1][OP_MAX + 1]) () = 
{
    {
        fsm_invalid,    // [sent = 0]       [recv = 0]
        recv_RRQ,       // [sent = 0]       [recv = OP_RRQ]
        recv_WRQ,       // [sent = 0]       [recv = OP_WRQ]
        fsm_invalid,    // [sent = 0]       [recv = OP_DATA]
        fsm_invalid,    // [sent = 0]       [recv = OP_ACK]
        fsm_invalid,    // [sent = 0]       [recv = OP_ERROR]
    },
    {
        fsm_invalid,    // [sent = OP_RRQ]  [recv = 0]
        fsm_invalid,    // [sent = OP_RRQ]  [recv = OP_RRQ]
        fsm_invalid,    // [sent = OP_RRQ]  [recv = OP_WRQ]
        fsm_invalid,    // [sent = OP_RRQ]  [recv = OP_DATA]
        fsm_invalid,    // [sent = OP_RRQ]  [recv = OP_ACK]
        fsm_invalid,    // [sent = OP_RRQ]  [recv = OP_ERROR]
    },
    {
        fsm_invalid,    // [sent = OP_WRQ]  [recv = 0]         
        fsm_invalid,    // [sent = OP_WRQ]  [recv = OP_RRQ]    
        fsm_invalid,    // [sent = OP_WRQ]  [recv = OP_WRQ]    
        fsm_invalid,    // [sent = OP_WRQ]  [recv = OP_DATA]   
        fsm_invalid,    // [sent = OP_WRQ]  [recv = OP_ACK]    
        fsm_invalid,    // [sent = OP_WRQ]  [recv = OP_ERROR]  
    },
    {
        fsm_invalid,    // [sent = OP_DATA] [recv = 0]          
        fsm_invalid,    // [sent = OP_DATA] [recv = OP_RRQ]     
        fsm_invalid,    // [sent = OP_DATA] [recv = OP_WRQ]     
        fsm_invalid,    // [sent = OP_DATA] [recv = OP_DATA]    
        recv_ACK,       // [sent = OP_DATA] [recv = OP_ACK]     
        fsm_error,      // [sent = OP_DATA] [recv = OP_ERROR]   
    },
    {
        fsm_invalid,    // [sent = OP_ACK] [recv = 0]          
        fsm_invalid,    // [sent = OP_ACK] [recv = OP_RRQ]     
        fsm_invalid,    // [sent = OP_ACK] [recv = OP_WRQ]     
        recv_DATA,      // [sent = OP_ACK] [recv = OP_DATA]    
        fsm_invalid,    // [sent = OP_ACK] [recv = OP_ACK]     
        fsm_error,      // [sent = OP_ACK] [recv = OP_ERROR]   
    },
    {
        fsm_invalid,    // [sent = OP_ERROR] [recv = 0]          
        fsm_invalid,    // [sent = OP_ERROR] [recv = OP_RRQ]     
        fsm_invalid,    // [sent = OP_ERROR] [recv = OP_WRQ]     
        fsm_invalid,    // [sent = OP_ERROR] [recv = OP_DATA]    
        fsm_invalid,    // [sent = OP_ERROR] [recv = OP_ACK]     
        fsm_error,      // [sent = OP_ERROR] [recv = OP_ERROR]   
    }
};


/*
 * Main loop of finite state machine.
 *
 * In a server, we're called after either an RRQ or a WRQ has been
 * received from the other side. In this case, the argument will be a 
 * 0 (since nothing has been sent) but the state table above handles 
 * this.
 * Return 0 on normal termination, -1 on timeout
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

    rtt_newpack(&rttinfo);      // initialize for a new packet

    for(; ;)
    {
        signal(SIGALRM, func_timeout);
        tout_flag = 0;
        alarm(rtt_start(&rttinfo));         // calc timeout & start timer

        if((nbytes = net_recv(recvbuff, MAXBUFF)) < 0)
        {
            if(tout_flag)
            {
                /*
                 * The receive time out. See if we've tried
                 * enough, and if so, return to caller.
                 */

                if(rtt_timeout(&rttinfo) < 0)
                    return -1;
            } else
            {
                D_printf("fsm_loop: net_recv error\n");
                exit(-1);
            }

            /* 
             * Retransmit the last packet
             */

            net_send(sendbuff, sendlen);
            continue;
        }

        alarm(0);       // stop signal timer
        tout_flag = 0;
        rtt_stop(&rttinfo); // stop RTT timer, calc new values

        if(nbytes < 4)
        {
            D_printf("fsm_loop: receive length = %d bytes\n", nbytes);
            exit(-1);
        }

        op_recv = ldshort(recvbuff);


        if(op_recv < OP_MIN || op_recv > OP_MAX)
        {
            D_printf("fsm_loop: invalid opcode received: %d\n", op_recv);
            exit(-1);
        }

        /*
         * We call the appropriate function, passing the address
         * of the received buffer and its length. These arguments
         * ignore the received-opcode, which we've already processed.
         *
         * We assume the called function will send a response to the 
         * other side. It is the called funciton's responsibility to
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
            return(0);
        }
    }
}

