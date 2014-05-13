#ifndef __PTRX_COMMON_H_INCLUDED__
#define __PTRX_COMMON_H_INCLUDED__

#define PTRX_OK         0
#define PTRX_ERROR      -1
#define PTRX_AGAIN      -2
#define PTRX_BUSY       -3
#define PTRX_DONE       -4
#define PTRX_DECLINED   -5
#define PTRX_ABORT      -6

#ifdef __DEBUG__
#define D_printf(fmt, arg...) fprintf(stderr, fmt, ##arg)
#else
#define D_printf(fmt, arg...)
#endif


#endif
