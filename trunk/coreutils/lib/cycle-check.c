/* help detect directory cycles efficiently */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cycle-check.h"

#define CC_MAGIC 9827862

void cycle_check_init(struct cycle_check_state* state)
{
    state->chdir_counter = 0;
    state->magic = CC_MAGIC;
}
