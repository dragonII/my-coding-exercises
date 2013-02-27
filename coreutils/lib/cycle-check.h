/* help detect directory cycles efficiently */

#ifndef CYCLE_CHECK_H
#define CYCLE_CHECK_H

#include <stdint.h>
#include <stdbool.h>
#include "dev-ino.h"
#include "same-inode.h"

struct cycle_check_state
{
    struct dev_ino dev_ino;
    uintmax_t chdir_counter;
    int magic;
};



void cycle_check_init(struct cycle_check_state* state);

#endif
