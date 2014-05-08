#ifndef __PTRX_STRING_H_INCLUDED__
#define __PTRX_STRING_H_INCLUDED__

#include <string.h>

typedef struct
{
    size_t          len;
    unsigned char  *data;
} ptrx_str_t;

#define ptrx_strlen(s)      strlen((const char *) s)


#endif
