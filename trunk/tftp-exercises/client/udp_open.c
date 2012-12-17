/*
 * Establish a UDP socket and optionally call connect() to set up
 * the server's address for future I/O.
 */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>

#include "error_cli.h"


/*
 * The following globals are available to the called, if desired.
 */

struct sockaddr_in  udp_srv_addr;       // server's Internet socket addr
struct sockaddr_in  udp_cli_addr;       // client's Internet socket addr
struct servent      udp_serv_info;      // from getservbyname()
struct hostent      udp_host_info;      // from gethostbyname()

/*
 * Return socket descriptor if OK, else -1 on error
 *
 *      host:       name of other system to communicate with
 *      service:    name of service being requested
 *                  can be NULL, if port > 0
 *      port:       if == 0, nothing special - use port# of service
 *                  if < 0, bind a local reserved port
 *                  if > 0, it's the port# server (host-byte-order)
 *      dontconn:   if == 0, call connect(), else don't
 */

int udp_open(char* host, char* service, int port, int dontconn)
{
    int fd;
    unsigned long inaddr;
    //char *host_err_str();
    struct servent  *sp;
    struct hostent  *hp;

    /*
     * Initialize the server's Internet address structure.
     * We'll store the actual 4-byte Internet address and the
     * 2-byte port# below.
     */

    bzero((char*)&udp_srv_addr, sizeof(udp_srv_addr));
    udp_srv_addr.sin_family = AF_INET;

    if(service != NULL)
    {
        if((sp = getservbyname(service, "udp")) == NULL)
        {
            err_ret("udp_open: unknown service: %s/udp", service);
            return -1;
        }
        udp_serv_info = *sp;
        if(port > 0)
            udp_srv_addr.sin_port = htons(port);    // caller's value
        else
            udp_srv_addr.sin_port = sp->s_port;     // service's value
    } else 
    {
        if(port <= 0)
        {
            err_ret("udp_open: must specify either service or port");
            return -1;
        }
        udp_srv_addr.sin_port = htons(port);
    }

    /*
     * First try to convert the host name as a dotted-decimal number.
     * Only if that failes do we call gethostbyname().
     */

    if((inaddr = inet_addr(host)) != INADDR_NONE)
    {
        bcopy((char*)&inaddr, (char*)&udp_srv_addr.sin_addr, sizeof(inaddr));
        udp_host_info.h_name = NULL;
    } else
    {
        if((hp = gethostbyname(host)) == NULL)
        {
            err_ret("udp_open: host name error: %s", host);
            return -1;
        }
        udp_host_info = *hp;    // found it by name
        bcopy(hp->h_addr, (char*)&udp_srv_addr.sin_addr, hp->h_length);
    }

    if(port < 0)
        err_quit("udp_open: reserved ports not implemented yet");

    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        err_ret("udp_open: can't create UDP socket");
        return -1;
    }

    /*
     * Bind any local address for us.
     */

    bzero((char*)&udp_cli_addr, sizeof(udp_cli_addr));
    udp_cli_addr.sin_family      = AF_INET;
    udp_cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_cli_addr.sin_port        = htons(0);
    if(bind(fd, (struct sockaddr*)&udp_cli_addr, sizeof(udp_cli_addr)) < 0)
    {
        err_ret("udp_open: bind error");
        close(fd);
        return -1;
    }

    /*
     * Call connect, if desired. This is used by most caller's,
     * as the peer shouldn't change. (TFTP is an exception)
     * By caling connect, the caller can call send() and recv().
     */

    if(dontconn == 0)
    {
        if(connect(fd, (struct sockaddr*)&udp_srv_addr,
                            sizeof(udp_srv_addr)) < 0)
        {
            err_ret("udp_open: connect error");
            return -1;
        }
    }
    return fd;
}
