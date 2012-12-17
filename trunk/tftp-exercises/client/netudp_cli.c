/*
 * TFTP network handling for UDP/IP connection.
 */

#include "defs_cli.h"
#include "error_cli.h"
#include "udp_open.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

extern int errno;

int sockfd = -1;
char openhost[MAXHOSTNAME] = { 0 };

extern struct sockaddr_in    udp_srv_addr;   // set by udp_open()
extern struct servent        udp_srv_info;   // set by udp_open()
static int    recv_first;

/*
 * Open the network connection.
 */

int net_open(char* host, char* service, int port)
{
    /*
     * Call udp_open() to create the socket. We tell udp_open to
     * not connect the socket, since we'll receive the first response
     * from a port that's different from where we send our first
     * datagram to.
     */

    if((sockfd = udp_open(host, service, port, 1)) < 0)
        return -1;

    D_printf("net_open: host %s, port# %d\n",
                inet_ntoa(udp_srv_addr.sin_addr),
                ntohs(udp_srv_addr.sin_port));

    strcpy(openhost, host);         // save the host's name
    recv_first = 1;                 // flag for net_recv()

    return 0;
}


/*
 * Close the network connection.
 */

void net_close()
{
    D_printf("net_close: host = %s, fd = %d\n", openhost, sockfd);

    close(sockfd);

    sockfd = -1;
}


/*
 * Send a record to the other end.
 * We use the sendto() system call, instead of send(), since the address
 * of the server changes after the first packet is sent.
 */

void net_send(char* buff, int len)
{
    int rc;

    D_printf("net_send: send %d bytes to host %s, port# %d\n",
                 len, inet_ntoa(udp_srv_addr.sin_addr),
                 ntohs(udp_srv_addr.sin_port));

    rc = sendto(sockfd, buff, len, 0, (struct sockaddr*)&udp_srv_addr,
                    sizeof(udp_srv_addr));

    if(rc < 0)
        err_quit("net_send: send error");
}


/*
 * Receive a record from the other end.
 */

int net_recv(char* buff, int maxlen)
{
    int nbytes;
    socklen_t fromlen;        // value-result parameter
    struct sockaddr_in  from_addr;  // actual addr of sender

    fromlen = sizeof(from_addr);
    nbytes = recvfrom(sockfd, buff, maxlen, 0, 
                (struct sockaddr*)&from_addr, &fromlen);

    /*
     * The recvfrom() system call can be interrupted by an alarm
     * interrupt, in case it times out. We just return -1 if the
     * system call was interrupted, and the caller must determine
     * if this is OK or not.
     */

    if(nbytes < 0)
    {
        if(errno == EINTR)
            return -1;
        else
        {
            err_sys("net_recv: recvfrom error");
        }
    }

    D_printf("net_recv: got %d bytes from host %s, port# %d\n",
                    nbytes, inet_ntoa(from_addr.sin_addr),
                    ntohs(from_addr.sin_port));

    /*
     * The TFTP client using UDP/IP has a funny requirement.
     * The problem is that UDP is being used for a 
     * "connection-oriented" protocol, which it wasn't really
     * designed for. Rather than tying up a single well-known
     * port number, the server changes its port after receiving
     * the first packet from a client.
     *
     * The first packet a client sends to the server (an RRQ or a WRQ)
     * must be sent to its well-known port number (69 for TFTP).
     * The server is then to choose some other port number for all
     * subsequent transfers. The recvfrom() call above will return
     * the server's current address. If the port number that we
     * sent the last packet to (udp_srv_addr.sin_port) is still equal to
     * the initial well-known port number (udp_serv_info.s_port), then we
     * must set the server's port for our next transmission to be
     * the port number from the recvfrom().
     *
     * Further more, after we have determined the port number that
     * we'll be receiving from, we can verify each datagram to make
     * certain its from the right place.
     */

    if(recv_first)
    {
        /*
         * This is the first received message.
         * The server's port should have changed.
         */

        if(udp_srv_addr.sin_port == from_addr.sin_port)
            err_quit("net_recv: first receive from port %d",
                        ntohs(from_addr.sin_port));

        udp_srv_addr.sin_port = from_addr.sin_port;     // save the new port# of the server

        recv_first = 0;
    } else if(udp_srv_addr.sin_port != from_addr.sin_port)
    {
        err_quit("net_recv: from port %d, expected from port %d",
                    ntohs(udp_srv_addr.sin_port),
                    ntohs(from_addr.sin_port));
    }

    return nbytes;      // return the actual length of the message
}
