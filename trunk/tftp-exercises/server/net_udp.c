/*
 * TFTP network handling for UDP/IP connection
 */

#include "defs.h"

#include <netinet/in.h>         // sockaddr_in
#include <netdb.h>              // servent
#include <arpa/inet.h>          // inet_not
#include <errno.h>
#include <strings.h>            // bzero
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net_udp.h"

extern int errno;
extern char recvbuff[];     // this is declared in initvars.c 

int tout_flag = 0;

int  sockfd = -1;

struct sockaddr_in  udp_srv_addr;
struct sockaddr_in  udp_cli_addr;

static int recv_nbytes = -1;
static int recv_first = 0;


/*
 * Initialize the network connection for the server
 */

void net_init(char* service, int port)
{
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        D_printf("net_init: cannot create datagram socket\n");
        exit(1);
    }

    /*
     * Bind local address so that any client can send to it
     */

    bzero((char*)&udp_srv_addr, sizeof(udp_srv_addr));
    udp_srv_addr.sin_family      = AF_INET;
    udp_srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_srv_addr.sin_port        = htons(port);

    if(bind(sockfd, (struct sockaddr*)&udp_srv_addr, sizeof(udp_srv_addr)) < 0)
    {
        D_printf("net_init: cannot bind local address, IP: %s, port: %d\n",
                 inet_ntoa(udp_srv_addr.sin_addr),
                 ntohs(udp_srv_addr.sin_port));
        exit(1);
    }

    D_printf("net_init: bind to IP: %s, port: %d\n",
             inet_ntoa(udp_srv_addr.sin_addr), 
             ntohs(udp_srv_addr.sin_port));
}

/*
 * Initialize the server's end, wait for a client's request to arrive
 */

int net_open(int inetdflag)
{
    int childpid, nbytes;

    recv_first  =  1;       // tell net_recv to save the address
    recv_nbytes = -1;       // tell net_recv to do the actual read
    nbytes = net_recv(recvbuff, MAXBUFF);

    /*
     * Fork a child process to handle the client's request.
     * The parent returns the child pid to the caller, which
     * is probably a concurrent server that'll call us again, to wait
     * for the next client request to this port;
     */

    if((childpid = fork()) < 0)
    {
        D_printf("server cannot fork\n");
        exit(1);
    } else if(childpid > 0) // parent
        return childpid;

    /*
     * Child process continues here.
     * First close the socket that is bound to this address
     * the parent will handle any further requests that arrive there.
     * We've already read the message that arrived for us to handle.
     */

    close(sockfd);
    errno = 0;  // in case it was set by close()

    /*
     * Create a new socket.
     * bind any local port# to the socket as our local address.
     * We don't connect(), since net_send() uses the send_to()
     * system call, specifying the destination address each time.
     */

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        D_printf("net_open: cannot create socket\n");
        exit(1);
    }

    bzero((char*)&udp_srv_addr, sizeof(udp_srv_addr));
    udp_srv_addr.sin_family      = AF_INET;
    udp_srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_srv_addr.sin_port        = htons(0);
    if(bind(sockfd, (struct sockaddr*)&udp_srv_addr, sizeof(udp_srv_addr)) < 0)
    {
        D_printf("net_open: bind error\n");
        exit(1);
    }

    /*
     * Now we'll set a special flag for net_recv(), so that
     * the next time it's called, it'll know the recvbuff[]
     * already has the received packet in it (from our call to
     * net_recv() above)
     */

    recv_nbytes = nbytes;

    return 0;
}

/*
 * Close a socket
 */

void net_close()
{
    D_printf("net_close: fd = %d\n", sockfd);
    close(sockfd);
    sockfd = -1;
}

/*
 * Send a record to the other end
 * The "struct sockaddr_in cli_addr" specifies the client's address
 */

void net_send(char* buff, int len)
{
    int rc;
    D_printf("net_send: send %d bytes to host %s, port# %d\n",
             len, inet_ntoa(udp_cli_addr.sin_addr),
             ntohs(udp_cli_addr.sin_port));

    rc = sendto(sockfd, buff, len, 0, (struct sockaddr*)&udp_cli_addr, 
                 sizeof(udp_cli_addr));

    if(rc != len)
    {
        D_printf("net_send: sendto error\n");
        exit(1);
    }
}

/*
 * Receive a record from the other end
 * We're called not only by the user, but also by net_open(),
 * to read the first datagram after a "connection" is established.
 */

int net_recv(char* buff, int maxlen)
{
    int                 nbytes;
    unsigned int        fromlen;        // value-result parameter
    extern int          tout_flag;   // set by SIGALRM
    struct sockaddr_in  from_addr;

    if(recv_nbytes >= 0)
    {
        /*
         * First message has been handled specially by net_open().
         * It's already been read into recvbuff[].
         */

        nbytes = recv_nbytes;
        recv_nbytes = -1;
        return nbytes;
    }
again:
    fromlen = sizeof(from_addr);
    nbytes = recvfrom(sockfd, buff, maxlen, 0, (struct sockaddr*)&from_addr, &fromlen);

    /*
     * The server can have its recvfrom() interrupted by either an
     * alarm timeout or by a SIGCLD interrupt. If it's timeout,
     * "tout_flag" will be set and we have to return to the caller
     * to let them determine if another receive should be initiated.
     * For a SIGCLD signal, we can restart the recvfrom() ourself.
     */

    if(nbytes < 0)
    {
        if(errno == EINTR)
        {
            if(tout_flag)
                return -1;

            errno = 0;      // assume SIGCLD
            goto again;
        }
        D_printf("net_recv: recvfrom error\n");
        exit(1);
    }

    D_printf("net_recv: got %d bytes from host %s, port#: %d\n",
             nbytes, inet_ntoa(from_addr.sin_addr),
             ntohs(from_addr.sin_port));

    /*
     * If "recv_first" is set, then we must save the received
     * address that recvfrom() stored in "from_addr" in the 
     * global "udp_cli_addr".
     */

    if(recv_first)
    {
        bcopy((char*)&from_addr, (char*)&udp_cli_addr, sizeof(from_addr));
        recv_first = 0;
    }
    
    /*
     * Make sure the message is from the expected client.
     */

    if(udp_cli_addr.sin_port != 0 &&
       udp_cli_addr.sin_port != from_addr.sin_port)
    {
        D_printf("net_recv: received from port %d, expected from port %d",
                 ntohs(from_addr.sin_port), ntohs(udp_cli_addr.sin_port));
        exit(1);
    }

    return nbytes;      // return the actual length of the message
}
