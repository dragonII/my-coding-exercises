#ifndef __PTRX_STRING_H__
#define __PTRX_STRING_H__

#include <stddef.h>

typedef struct
{
    size_t          len;
    unsigned char   *data;
} ptrx_str_t;

typedef struct
{
    unsigned    len:28;

    unsigned    valid:1;
    unsigned    no_cacheable:1;
    unsigned    not_found:1;
    unsigned    escape:1;

    unsigned char *data;
} ptrx_variable_value_t;

#define ptrx_string(str)    { sizeof(str) - 1, (unsigned char *)str }
#define ptrx_null_string    { 0, NULL }

#define ptrx_strlen(s)      strlen((const char *) s)

#define ptrx_memcpy(dst, src, n)    (void)memcpy(dst, src, n)
#define ptrx_cpymem(dst, src, n)    (((unsigned char *)memcpy(dst, src, n)) + (n))

unsigned char * ptrx_cpystrn(unsigned char *dst, unsigned char *src, size_t n);


#endif
