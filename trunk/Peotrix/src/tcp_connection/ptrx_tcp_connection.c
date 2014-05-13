#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <strings.h>     /* for bzero */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int	errno;

#include <common/ptrx_common.h>
#include <ptrx_tcp_connection.h>

int	    sockfd = -1;				/* fd for socket of server */

/*
 * Close the network connection.  Used by client and server.
 */

void ptrx_net_close(ptrx_tcp_conn_t *tcp_conn_info)
{
    close(sockfd);
    
    sockfd = -1;
}

/*
 * Send a record to the other end.  Used by client and server.
 * With a stream socket we have to preface each record with its length,
 * since TFTP doesn't have a record length as part of each record.
 * We encode the length as a 2-byte integer in network byte order.
 */

void ptrx_net_send(char *buff, int len, ptrx_tcp_conn_t *tcp_conn_info)
{
    register int	rc;
    short		templen;
    
    D_printf("net_send: sent %d bytes", len);
    
    templen = htons(len);
    rc = write(sockfd, (char *) &templen, sizeof(short));
    if (rc != sizeof(short))
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_ERROR,
                0, "writen error of lenth prefix");
        return;
    }
    
    rc = write(sockfd, buff, len);
    if (rc != len)
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_ERROR,
                0, "writen error");
        return;
    }
}


int				/* return #bytes in packet, or -1 on EINTR */
ptrx_net_recv(char *buff, int maxlen, ptrx_tcp_conn_t *tcp_conn_info)
{
    register int    nbytes;
    short           templen;	/* value-result parameter */

again1:
    if ( (nbytes = read(sockfd, (char *) &templen, sizeof(short))) < 0) 
    {
        if (errno == EINTR) 
        {
            errno = 0;          /* assume SIGCLD */
            goto again1;
        } else
        {
            ptrx_log_stderr(tcp_conn_info->log, PTRX_ERROR,
                    0, "read error");
            return PTRX_ERROR;
        }
    }
    if (nbytes != sizeof(short))
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_ERROR,
                    0, "error in read of length prefix");
        return PTRX_ERROR;
    }
    
    templen = ntohs(templen);		/* #bytes that follow */
    if (templen > maxlen)
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_ERROR,
                        0, "record length too large");
        return PTRX_ERROR;
    }

again2:
    if ( (nbytes = read(sockfd, buff, templen)) < 0) 
    {
        if (errno == EINTR) 
        {
            errno = 0;		/* assume SIGCLD */
            goto again2;
        } else
        {
            ptrx_log_stderr(tcp_conn_info->log, PTRX_ERROR,
                        0, "read error");
            return PTRX_ERROR;
        }
    }
    if (nbytes != templen)
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_ERROR,
                        0, "read error");
        return PTRX_ERROR;
    }
    
    D_printf("net_recv: got %d bytes", nbytes);
    
    return(nbytes);		/* return the actual length of the message */
}



/*
 * Initialize the network connection for the server, when it has *not*
 * been invoked by inetd.
 *      int      port: if nonzero, this is the port to listen on;
 *                     overrides the standard port for the service 
 */

int ptrx_net_init(int port, ptrx_tcp_conn_t *tcp_conn_info)
{
    struct sockaddr_in  tcp_srv_addr;	/* set by tcp_open() */
    /*
    * We weren't started by a master daemon.
    * We have to create a socket ourselves and bind our own 
    * address to it.
    */
    
    bzero((char *) &tcp_srv_addr, sizeof(tcp_srv_addr));
    tcp_srv_addr.sin_family      = AF_INET;
    tcp_srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (port <= 0) 
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_LOG_ERR,
                    0, "tcp_open: must specify port");
        return PTRX_ERROR;
    }
    tcp_srv_addr.sin_port = htons(port);
    tcp_conn_info->port = port;

    /*
     * Create the socket and Bind our local address so that any
     * client can send to us.
     */
    
    //if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    if ( (tcp_conn_info->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_LOG_ERR,
                    0, "net_init: can't create stream socket");
        return PTRX_ERROR;
    }
    
    if(fcntl(tcp_conn_info->sockfd, F_SETFL, O_NONBLOCK) != 0)
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_LOG_ERR,
                    0, "sockfd setnonblocking failed");
        return PTRX_ERROR;
    }

    if (bind(tcp_conn_info->sockfd, (struct sockaddr *) &tcp_srv_addr,
                sizeof(tcp_srv_addr)) < 0)
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_LOG_ERR,
                    0, "net_init: can't bind local address");
        return PTRX_ERROR;
    }
    
    /*
     * And set the listen parameter, telling the system that we're
     * ready  to accept incoming connection requests.
     */
    
    listen(tcp_conn_info->sockfd, 20);

    return PTRX_OK;
}

/*
 * Initiate the server's end.
 * We are passed a flag that says whether or not we were started
 * by a "master daemon," such as the inetd program under 4.3BSD.
 * A master daemon will have already waited for a message to arrive
 * for us, and will have already set up the connection to the client.
 * If we weren't started by a master daemon, then we must wait for a
 * client's request to arrive.
 *
 * Return value:
 *      PTRX_OK   => returned by parent, tell caller to wait for next
 *      newsockfd => returned by child, tell caller to handle
 *
 */

int  ptrx_net_open(ptrx_tcp_conn_t *tcp_conn_info)
{
    register int        newsockfd, childpid;
    unsigned int        clilen;
    struct sockaddr_in  tcp_cli_addr;	/* set by accept() */
    
    /*
     * For the concurrent server we have to wait for 
     * a connection request to arrive,
     * then fork a child to handle the client's request.
     * Beware that the accept() can be interrupted, such as by
     * a previously spawned child process that has terminated
     * (for which we caught the SIGCLD signal).
     *
     * TODO: process or thread?
     */

again:
    clilen = sizeof(tcp_cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &tcp_cli_addr, &clilen);
    if (newsockfd < 0) 
    {
        if (errno == EINTR) 
        {
            errno = 0;
            goto again;	/* probably a SIGCLD that was caught */
        }
        ptrx_log_stderr(tcp_conn_info->log, PTRX_LOG_ERR,
                    0, "accept error");
        return PTRX_ERROR;
    }

    /*
     * Fork a child process to handle the client's request.
     * The parent returns the child pid to the caller, which is
     * probably a concurrent server that'll call us again, to wait
     * for the next client request to this well-known port.
     */
    
    if ( (childpid = fork()) < 0)
    {
        ptrx_log_stderr(tcp_conn_info->log, PTRX_LOG_ERR,
                    0, "server can't fork");
    } else if(childpid > 0) 
    {		/* parent */
        close(newsockfd);   /* close new connection */
        return PTRX_OK;
    }

    /*
    * Child process continues here.
    * First close the original socket so that the parent
    * can accept any further requests that arrive there.
    * Then set "sockfd" in our process to be the descriptor that
    * we are going to process.
    */
    
    close(tcp_conn_info->sockfd);
    tcp_conn_info->sockfd = newsockfd;
    
    return newsockfd;       /* return to process the connection */
}

