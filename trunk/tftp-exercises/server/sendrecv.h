#ifndef __SENDRECV_H__
#define __SENDRECV_H__


int recv_RRQ(char* ptr, int nbytes);
int recv_WRQ(char* ptr, int nbytes);
int recv_ACK(char* ptr, int nbytes);
int recv_DATA(char* ptr, int nbytes);

#endif
