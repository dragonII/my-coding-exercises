/* a simple ring buffer */

#include "i-ring.h"

#include <stdlib.h>

void i_ring_init(I_ring* ir, int default_val)
{
    int i;
    ir->ir_empty = true;
    ir->ir_front = 0;
    ir->ir_back = 0;
    for(i = 0; i < I_RING_SIZE; i++)
        ir->ir_data[i] = default_val;
    ir->ir_default_val = default_val;
}

bool i_ring_empty(I_ring* ir)
{
    return ir->ir_empty;
}

int i_ring_push(I_ring* ir, int val)
{
    unsigned int dest_idx = (ir->ir_front + !ir->ir_empty) % I_RING_SIZE;
    int old_val = ir->ir_data[dest_idx];
    ir->ir_data[dest_idx] = val;
    ir->ir_front = dest_idx;
    if(dest_idx == ir->ir_back)
        ir->ir_back = (ir->ir_back + !ir->ir_empty) % I_RING_SIZE;
    ir->ir_empty = false;
    return old_val;
}

int i_ring_pop(I_ring* ir)
{
    int top_val;
    if(i_ring_empty(ir))
        abort();
    top_val = ir->ir_data[ir->ir_front];
    ir->ir_data[ir->ir_front] = ir->ir_default_val;
    if(ir->ir_front == ir->ir_back)
        ir->ir_empty = true;
    else
        ir->ir_front = ((ir->ir_front + I_RING_SIZE - 1) % I_RING_SIZE);
    return top_val;
}
