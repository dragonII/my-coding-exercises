#ifndef __NETUDP_CLIENT_H__
#define __NETUDP_CLIENT_H__


int  net_open(char* host, char* service, int port);
void net_close();
void net_send(char* buff, int len);
int  net_recv(char* buff, int maxlen);

#endif
