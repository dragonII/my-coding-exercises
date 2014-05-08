/* Application entry -- main loop */


#include <peotrix.h>
#include <sys/epoll.h>

#include <ptrx_log.h>

int main(int argc, char **argv)
{
    unsigned int    port = PORT_NUM;
    int             epoolfd;
    int             rc;

    ptrx_log_t  *log;

    rc = ptrx_log_init(log);

    ///* create epoll */
    //epoolfd = epoll_create(MAX_EPOLL_SIZE);

    //if(epoolfd <= 0)


