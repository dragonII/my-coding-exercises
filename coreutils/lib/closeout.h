#ifndef _CLOSEOUT_H
#define _CLOSEOUT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void close_stdout_set_file_name(const char* file);
void close_stdout_set_ignore_EPIPE(bool ignore);
void close_stdout(void);


#ifdef __cplusplus
}
#endif


#endif
