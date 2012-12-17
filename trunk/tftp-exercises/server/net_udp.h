#ifndef __NET_UDP_H__
#define __NET_UDP_H__

void net_init(char* service, int port);
int  net_open(int inetdflag);
void net_close();
void net_send(char* buff, int len);
int  net_recv(char* buff, int maxlen);

#endif
