/* Return the canonical absolute name of a given file */

#ifndef CANONICALIZE_H
#define CANONICALIZE_H

#include <stdlib.h>  /* for canonicalize_file_name */

enum canonicalize_mode_t
{
    /* All components must exist */
    CAN_EXISTING = 0,

    /* All components excluding last one must exist */
    CAN_ALL_BUT_LAST = 1,

    /* No requirements on components existence */
    CAN_MISSING = 2
};

typedef enum canonicalize_mode_t canonicalize_mode_t;

char* canonicalize_filename_mode(char* name, canonicalize_mode_t can_mode);

#endif
