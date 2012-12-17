/*
 * Timer routines for round-trip timing of datagrams.
 *
 * rtt_init()       Called to initialize everything for a given "connection"
 * rtt_newpack()    Called before each new packet is transmitted on
 *                      a "connection" Initializaes retransmit counter to 0
 * rtt_start()      Called before each packet either transmitted or
 *                      retransmitted. Calculates the timeout value for
 *                      the packet and starts the timer to calculate the RTT
 * rtt_stop()       Called after a packet has been received.
 * rtt_timeout()    Called after a timeout has occured. Tells you
 *                      if you should retransmit again, or give up
 *
 * The differnce between rtt_init() and rtt_newpack() is that the former
 * knows nothing about the "connection", while the latter makes use of
 * previous RTT information for a given "connection"
 */

#include "rtt.h"
#include "defs.h"

#include <stdlib.h>

int exp_backoff[RTT_MAXNREXMT + 1] = {1, 2, 4, 8, 16};
    /* indexed by rtt_nrexmt: 0, 1, 2, ..., RTT_MAXNREXMT.
        [0] entry (==1) is not used;
        [1] entry (==2) is used the second time a packet is sent;
        ... */

int rtt_d_flag = 0;     // can be set nonzero by caller

/*
 * Initialize an RTT structure.
 * This function is called before the first packet is transmitted.
 */

void rtt_init(struct rtt_struct* ptr)
{
    ptr->rtt_rtt    = 0;
    ptr->rtt_srtt   = 0;
    ptr->rtt_rttdev = 1.5;
        /* first timeout at (srtt + (2 * rttdev)) = 3 seconds */
    ptr->rtt_nxtrto = 0;
}

/*
 * Initialize the retransmit counter before a packet is transmitted
 * the first time..
 */

void rtt_newpack(struct rtt_struct* ptr)
{
    ptr->rtt_nrexmt = 0;
}

/*
 * Start our RTT timer.
 * This should be called right before the alarm() call before a packet 
 * is received. We calculate the integer alarm() value to use for the
 * timeout (RTO) and return it as the value of the function.
 */

int rtt_start(struct rtt_struct* ptr)
{
    int rexmt;

    if(ptr->rtt_nrexmt > 0)
    {
        /*
         * This is a retransmission. No need to obtain the
         * starting time, as we won't use the RTT for anything.
         * Just apply the exponential back off and return.
         */
        ptr->rtt_currto *= exp_backoff[ptr->rtt_nrexmt];
        return ptr->rtt_currto;
    }
    
    if(gettimeofday(&ptr->time_start, (struct timezone *)0) < 0)
    {
        D_printf("rtt_start: gettimeofday() error\n");
        exit(1);
    }

    if(ptr->rtt_nxtrto > 0)
    {
        /*
         * This is the first transmission of a packet *and* the
         * last packet had to be retransmitted. Therefore, we'll
         * use the final RTO for the previous packet as the
         * starting RTO for this packet. If that RTO is OK for
         * this packet, then we'll start updating the RTT estimators.
         */

        ptr->rtt_currto = ptr->rtt_nxtrto;
        ptr->rtt_nxtrto = 0;
        return ptr->rtt_currto;
    }

    /*
     * Calculate the timeout value based on current estimators:
     *  smoothed RTT plus twice the deviation.
     */

    rexmt = (int) (ptr->rtt_srtt + (2.0 * ptr->rtt_rttdev) + 0.5);
    if(rexmt < RTT_RXTMIN)
        rexmt = RTT_RXTMIN;
    else if(rexmt > RTT_RXTMAX)
        rexmt = RTT_RXTMAX;
    
    ptr->rtt_currto = rexmt;
    
    return ptr->rtt_currto;
}

/*
 * A response was received.
 * Stop the timer and update the appropriate values in the structure
 * based on this packet's RTT. We calculate the RTT, then update the
 * smoothed RTT and the RTT variance.
 * This function should be called right after turning off the
 * timer with alarm(0), or right after a timeout occurs.
 */

void rtt_stop(struct rtt_struct* ptr)
{
    double start, stop, err;

    if(ptr->rtt_nrexmt > 0)
    {
        /*
         * The response was for a packet that has been retransmitted.
         * We don't know which transmission the reponse corresponds to.
         * We didn't record the start time in rtt_start(), so there's
         * no need to record the stop time here. We also don't 
         * update our estimators.
         * We do, however, save the RTO corresponding to this
         * response, and it'll be used for the next packet.
         */

        ptr->rtt_nxtrto = ptr->rtt_currto;
        return;
    }
    ptr->rtt_nxtrto = 0;    // for next call to rtt_start

    if(gettimeofday(&ptr->time_stop, (struct timezone*)0) < 0)
    {
        D_printf("rtt_stop: gettimeofday() error\n");
        exit(1);
    }

    start = ((double)ptr->time_start.tv_sec) * 1000000.0
                + ptr->time_start.tv_usec;
    stop  = ((double)ptr->time_stop.tv_sec) * 1000000.0
                + ptr->time_stop.tv_usec;

    ptr->rtt_rtt = (stop - start) / 1000000.0;  // in seconds;

    /*
     * Update our estimators of RTT and mean deviation of RTt.
     * See Jacobson's SIGCOMM '88 paper, Appendix A, for the details.
     * This appendix also contains a fixed-point, integer implemetation
     * (that is actually used in all the post-4.3 TCP code).
     * We'll use floating point here for simplicity.
     *
     * First
     *  err = (rtt - old_srtt) = difference between this measured value
     *               and current estimators.
     * and
     *  new_srtt = old_srtt*7/8 + rtt/8
     * Then
     *  new_srtt = old_srtt + err / 8
     *
     * Also
     *  new_rttdev = old_rttdev + (|err| - old_rttdev) / 4
     */

    err = ptr->rtt_rtt - ptr->rtt_srtt;
    ptr->rtt_srtt += err / 8;

    if(err < 0.0)
        err = -err;     // |err|

    ptr->rtt_rttdev += (err - ptr->rtt_rttdev) / 4;
}

/*
 * A timeout has occured.
 * This function should be called right after the timeout alarm occurs.
 * Return -1 if it's time to give up, else return 0.
 */

int rtt_timeout(struct rtt_struct* ptr)
{
    rtt_stop(ptr);

    if(++ptr->rtt_nrexmt > RTT_MAXNREXMT)
        return -1;
    return 0;
}

/*
 * Print debugging information on stderr, if the "rtt_d_flag" is nonzero
 */

void rtt_debug(struct rtt_struct* ptr)
{
    if(rtt_d_flag == 0)
        return;

    fprintf(stderr, "rtt = %.5f, srtt = %.3f, rttdev = %.3f, currto = %d\n",
            ptr->rtt_rtt, ptr->rtt_srtt, ptr->rtt_rttdev,
            ptr->rtt_currto);
    fflush(stderr);
}
