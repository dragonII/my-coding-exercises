#ifndef __RTT_H__
#define __RTT_H__

/*
 * Definitions for RTT timing
 */

#include <stdio.h>
#include <sys/time.h>


/*
 * Structure to contain everything needed for RTT timing.
 * One of these required per socket being timed.
 * The caller allocates this structure, then passes its address to
 * all the rtt_XXX() functions.
 */

struct rtt_struct {
    float rtt_rtt;      // most recent round-trip time (RTT), seconds
    float rtt_srtt;     // smoothed round-trip time (SRTT), seconds
    float rtt_rttdev;   // smoothed mean deviation, seconds
    short rtt_nrexmt;   // #times retransmitted: 0, 1, 2 ...
    short rtt_currto;   // current retransmit timeout (RTO), seconds
    short rtt_nxtrto;   // retransmit timeout for next packet, if nonzero

    struct timeval  time_start; // for elapsed time
    struct timeval  time_stop;  // for elapsed time
};

#define RTT_RXTMIN      2   // min retransmit timeout value, seconds
#define RTT_RXTMAX    120   // max retransmit timeout value, seconds
#define RTT_MAXNREXMT   4   // max #times to retransmit: must also
                            // change exp_backoff[] if this changes.

extern int rtt_d_flag;      // can be set nonzero by caller for addl info


void rtt_init(struct rtt_struct* ptr);
void rtt_newpack(struct rtt_struct* ptr);
int  rtt_start(struct rtt_struct* ptr);
void rtt_stop(struct rtt_struct* ptr);
int  rtt_timeout(struct rtt_struct* ptr);
void rtt_debug(struct rtt_struct* ptr);

#endif
