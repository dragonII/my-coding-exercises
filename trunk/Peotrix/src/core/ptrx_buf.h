#ifndef __PTRX_BUF_H__
#define __PTRX_BUF_H__

#include <ptrx_files.h>

typedef void *          ptrx_buf_tag_t;

typedef struct ptrx_buf_s      ptrx_buf_t;
struct ptrx_buf_s
{
    unsigned char       *pos;
    unsigned char       *last;
    off_t               file_pos;
    off_t               file_last;

    unsigned char       *start;     /* start of buffer */
    unsigned char       *end;       /* end of buffer */
    ptrx_buf_tag_t      tag;
    ptrx_file_t         *file;
    ptrx_buf_t          *shadow;

    /* the buf's content could be changed */
    unsigned            temporary:1;

    /* 
     * The buf's content is in a memory cache or in a read only 
     * memory and must not be changed 
     */
    unsigned            memory:1;

    /* the buf's content is mmap()ed and must not be changed */
    unsigned            mmap:1;

    unsigned            recycled:1;
    unsigned            in_file:1;
    unsigned            flush:1;
    unsigned            sync:1;
    unsigned            last_buf:1;
    unsigned            last_in_chain:1;

    unsigned            last_shadow:1;
    unsigned            temp_file:1;

    /* STUB */int       num;
};

typedef struct ptrx_chain_s    ptrx_chain_t;
struct ptrx_chain_s
{
    ptrx_buf_t      *buf;
    ptrx_chain_t    *next;
};




#endif
