#ifndef __PTRX_STRING_H__
#define __PTRX_STRING_H__

#include <stddef.h>

typedef struct
{
    size_t          len;
    unsigned char   *data;
} ptrx_str_t;

#define ptrx_string(str)    { sizeof(str) - 1, (unsigned char *)str }
#define ptrx_null_string    { 0, NULL }


#endif
