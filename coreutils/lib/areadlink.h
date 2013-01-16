/* Read symbolic links without size limitation */

#ifndef AREADLINK_H
#define AREADLINK_H

#include <sys/types.h>


char* areadlink_with_size(char* file, size_t size);


#endif
