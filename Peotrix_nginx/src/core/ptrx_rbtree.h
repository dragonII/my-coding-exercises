#ifndef __PTRX_RBTREE_H__
#define __PTRX_RBTREE_H__

typedef unsigned int                ptrx_rbtree_key_t;
typedef struct ptrx_rbtree_node_s   ptrx_rbtree_node_t;

struct ptrx_rbtree_node_s
{
    ptrx_rbtree_key_t       key;
    ptrx_rbtree_node_t      *left;
    ptrx_rbtree_node_t      *right;
    ptrx_rbtree_node_t      *parent;
    unsigned char           color;
    unsigned char           data;
};





#endif
