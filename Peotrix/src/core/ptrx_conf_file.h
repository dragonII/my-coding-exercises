#ifndef __PTRX_CONF_H__
#define __PTRX_CONF_H__

#include <ptrx_string.h>
#include <ptrx_cycle.h>


//typedef struct ptrx_open_file_s ptrx_open_file_t;
//struct ptrx_open_file_s
//{
//    int             fd;
//    ptrx_str_t      name;
//    unsigned char   *buffer;
//    unsigned char   *pos;
//    unsigned char   *last;
//};


int ptrx_conf_full_name(ptrx_cycle_t *cycle,
                        ptrx_str_t   *name,
                        unsigned int  conf_prefix);



#endif
