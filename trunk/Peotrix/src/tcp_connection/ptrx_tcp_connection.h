#ifndef __PTRX_TCP_CONN_H_INCLUDED__
#define __PTRX_TCP_CONN_H_INCLUDED__

#include <log/ptrx_log.h>

typedef struct ptrx_tcp_conn_s
{
    int sockfd;
    int port;

    ptrx_log_t *log;
} ptrx_tcp_conn_t;


void ptrx_net_init(ptrx_tcp_conn_t *tcp_conn_info);
int  ptrx_net_open(ptrx_tcp_conn_t *tcp_conn_info);
void ptrx_net_close(ptrx_tcp_conn_t *tcp_conn_info);
void ptrx_net_send(char *buff, int len, ptrx_tcp_conn_t *tcp_conn_info);
int  ptrx_net_recv(char *buff, int maxlen, ptrx_tcp_conn_t *tcp_conn_info);



#endif
