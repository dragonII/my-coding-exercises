#ifndef __PTRX_CONF_H__
#define __PTRX_CONF_H__

#include "ptrx_string.h"


struct ptrx_open_file_s
{
    int             fd;
    ptrx_str_t      name;
    unsigned char   *buffer;
    unsigned char   *pos;
    unsigned char   *last;
};

typedef struct ptrx_open_file_s ptrx_open_file_t;



#endif
