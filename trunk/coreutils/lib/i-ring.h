/* definitions for a simple ring buffer */

#ifndef I_RING_H
#define I_RING_H

#include <stdbool.h>
#include "verify.h"

enum { I_RING_SIZE = 4 };

verify(1 <= I_RING_SIZE);

/* When ir_empty is true, the ring is empty.
   Otherwise, ir_data[B..F] are defined, where B..F is the contiguous
   range of indices, modulo I_RING_SIZE, from back to front, inclusive.
   Undefined elements of ir_data are always set to ir_default_val.
   Popping from an empty ring aborts.
   Pushing onto a full ring returns the displaces value.
   An empty tring has F==B and ir_empty == true.
   A ring with one entry still has F==B, but now ir_empty == false. */

struct I_ring
{
    int ir_data[I_RING_SIZE];
    int ir_default_val;
    unsigned int ir_front;
    unsigned int ir_back;
    bool ir_empty;
};

typedef struct I_ring I_ring;

void i_ring_init(I_ring*, int);
bool i_ring_empty(I_ring*);
int  i_ring_push(I_ring*, int);
int  i_ring_pop(I_ring*);


#endif
