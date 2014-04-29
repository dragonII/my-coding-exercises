#ifndef __PTRX_CONNECTION_H__
#define __PTRX_CONNECTION_H__

#include <ptrx_event.h>
#include <ptrx_socket.h>
#include <ptrx_queue.h>
#include <ptrx_atomic.h>

typedef struct ptrx_connection_s   ptrx_connection_t;

typedef ssize_t (*ptrx_recv_pt)(ptrx_connection_t *c, unsigned char *buf, size_t size);
typedef ssize_t (*ptrx_send_pt)(ptrx_connection_t *c, unsigned char *buf, size_t size);
typedef ssize_t (*ptrx_recv_chain_pt)(ptrx_connection_t *c, ptrx_chain_t *in);
typedef ptrx_chain_t *(*ptrx_send_chain_pt)(ptrx_connection_t *c, ptrx_chain_t *in, off_t limit);
typedef void (*ptrx_connection_handler_pt)(ptrx_connection_t *c);

typedef struct ptrx_listening_s ptrx_listening_t;

struct ptrx_listening_s
{
    ptrx_socket_t               fd;

    struct sockaddr             *sockaddr;
    socklen_t                   socklen;    /* size of sockaddr */
    size_t                      addr_text_max_len;
    ptrx_str_t                  addr_text;

    int                         type;

    int                         backlog;
    int                         rcvbuf;
    int                         sndbuf;

    /* handler of accepted connection */
    ptrx_connection_handler_pt  handler;

    void                        *servers; /* array of ptrx_http_in_addr_t, for example */

    ptrx_log_t                  log;
    ptrx_log_t                  *logp;

    size_t                      pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t                      post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ptrx_msec_t                 post_accept_timeout;

    ptrx_listening_t            *previous;
    ptrx_connection_t           *connection;

    unsigned                    open:1;
    unsigned                    remain:1;
    unsigned                    ignore:1;

    unsigned                    bound:1;   /* already bound */
    unsigned                    inherited:1;    /* inherited from previous process */
    unsigned                    nonblocking_accept:1;
    unsigned                    listen:1;
    unsigned                    nonblocking:1;
    unsigned                    shared:1;   /* shared between threads or processes */
    unsigned                    addr_ntop:1;
};

struct ptrx_connection_s
{
    void                *data;
    ptrx_event_t        *read;
    ptrx_event_t        *write;

    ptrx_socket_t       fd;

    ptrx_recv_pt        recv;
    ptrx_send_pt        send;
    ptrx_recv_chain_pt  recv_chain;
    ptrx_send_chain_pt  send_chain;

    ptrx_listening_t    *listening;

    off_t               sent;

    ptrx_log_t          *log;

    ptrx_pool_t         *pool;

    struct sockaddr     *sockaddr;
    socklen_t           socklen;
    ptrx_str_t          addr_text;

#if (PTRX_SSL)
    ptrx_ssl_connection_t   *ssl
#endif

    struct sockaddr     *local_sockaddr;

    ptrx_buf_t          *buffer;

    ptrx_queue_t        queue;

    ptrx_atomic_uint_t  number;

    unsigned int        request;

    unsigned            buffered:8;

    unsigned            log_error:3;    /* ptrx_connection_log_error_e */

    unsigned            single_connection:1;
    unsigned            unexpected_eof:1;
    unsigned            timeout:1;
    unsigned            error:1;
    unsigned            destroyed:1;

    unsigned            idle:1;
    unsigned            reusable:1;
    unsigned            close:1;

    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    unsigned            tcp_nodelay:2;  /*ptrx_connection_tcp_nodelay_e*/
    unsigned            tcp_nopush:2;   /*ptrx_connection_tcp_nopush_e*/

    ptrx_atomic_t       lock;
};



#endif
