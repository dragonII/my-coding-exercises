/* Return a string describing the type of a file */

#ifndef FILE_TYPE_H
#define FILE_TYPE_H

#include <sys/types.h>
#include <sys/stat.h>

const char* file_type(struct stat*);

#endif
