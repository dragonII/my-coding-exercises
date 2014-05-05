#ifndef __PTRX_CORE_H__
#define __PTRX_CORE_H__


#define LF      (unsigned char)10
#define CR      (unsigned char)13
#define CRLF    "\0x0d\x0a"

#define PTRX_PREFIX     "/usr/local/PreTrix/"
#define PTRX_CONF_PATH  "conf/ptrx.conf"
#define PTRX_PID_PATH   "log/peotrix.pid"

#define PTRX_OK         0
#define PTRX_ERROR      -1
#define PTRX_AGAIN      -2
#define PTRX_BUSY       -3
#define PTRX_DONE       -4
#define PTRX_DECLINED   -5
#define PTRX_ABORT      -6

#endif
