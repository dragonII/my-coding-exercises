#ifndef __PTRX_TCP_CONN_H_INCLUDED__
#define __PTRX_TCP_CONN_H_INCLUDED__

void ptrx_net_init(char *service, int port);
int  ptrx_net_open(int inetdflag);
void ptrx_net_close();
void ptrx_net_send(char *buff, int len);
int  ptrx_net_recv(char *buff, int maxlen);



#endif
