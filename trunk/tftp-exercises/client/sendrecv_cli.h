#ifndef __SENDRECV_CLI_H__
#define __SENDRECV_CLI_H__


void send_RQ(int opcode, char* fname, int mode);
int  recv_RQERR(char* ptr, int nbytes);
void send_ACK(int blocknum);
void send_DATA(int blocknum, int nbytes);
int  recv_DATA(char* ptr, int nbytes);
int  recv_ACK(char* ptr, int nbytes);



#endif
