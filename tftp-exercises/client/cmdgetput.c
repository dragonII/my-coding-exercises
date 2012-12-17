/* 
 * File get/put processing
 *
 * This is the way the client side gets started - either the user
 * wants to get a file (generates a RRQ command to the server)
 * or the user wants to put a file (generates a WRQ command to the 
 * server). Once either the RRQ or the WRQ command is sent,
 * the finite state machine takes over the transmission.
 */

#include "defs_cli.h"
#include "error_cli.h"
#include "file_cli.h"
#include "netudp_cli.h"
#include "fsm_cli.h"
#include "sendrecv_cli.h"

#include <sys/time.h>
#include <sys/resource.h>

/*
 * Execute a get command - read a remote file and store on the local system.
 */

static struct timeval st_start, st_stop;
static struct rusage  ru_start, ru_stop;

/*
 * Start the timer.
 * We don't return anything to the caller, we just store some 
 * information for the stop timer routine to access.
 */

void t_start()
{
    if(gettimeofday(&st_start, (struct timezone*)0) < 0)
        err_sys("t_start: gettimeofday() error");

    if(getrusage(RUSAGE_SELF, &ru_start) < 0)
        err_sys("t_start: getrusage() error");
}


/*
 * Stop the timer and save the appropriate information
 */

void t_stop()
{
    if(getrusage(RUSAGE_SELF, &ru_stop) < 0)
        err_sys("r_stop: getrusage() error");
    if(gettimeofday(&st_stop, (struct timezone*)0) < 0)
        err_sys("r_stop: gettimeofday() error");
}


/*
 * Return the real (elapsed) time in seconds
 */

double t_getrtime()
{
    double start = ((double)st_start.tv_sec) * 1000000.0
                        + st_start.tv_usec;

    double stop  = ((double)st_stop.tv_sec) * 1000000.0
                        + st_stop.tv_usec;

    double seconds = (stop - start) / 1000000.0;

    return seconds;
}
void do_get(char* remfname, char* locfname)
{
    if((localfp = file_open(locfname, "w", 1)) == NULL)
    {
        err_ret("cannot fopen %s for writing", locfname);
        return;
    }

    if(net_open(hostname, TFTP_SERVICE, port) < 0)
        return;

    totnbytes = 0;
    t_start();          // start timer for statistics

    send_RQ(OP_RRQ, remfname, modetype);

    fsm_loop(OP_RRQ);

    t_stop();           // stop timer for statistics

    net_close();

    file_close(localfp);

    // print statistics
    printf("Received %ld bytes in %.1f seconds\n", totnbytes, t_getrtime());
}


/*
 * Execute a put command - send a local file to the remote system
 */

void do_put(char* remfname, char* locfname)
{
    if((localfp = file_open(locfname, "r", 0)) == NULL)
    {
        err_ret("cannot fopen %s for reading", locfname);
        return;
    }

    if(net_open(hostname, TFTP_SERVICE, port) < 0)
        return;

    totnbytes = 0;
    t_start();          // start timer for statistics

    lastsend = MAXDATA;
    send_RQ(OP_WRQ, remfname, modetype);

    fsm_loop(OP_WRQ);

    t_stop();           // stop timer for statistics

    net_close();

    file_close(localfp);

    printf("Send %ld bytes in %.lf seconds\n", totnbytes, t_getrtime());

    return;
}
