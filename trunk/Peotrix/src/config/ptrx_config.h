#ifndef __PTRX_CONFIG_H_INCLUDED__
#define __PTRX_CONFIG_H_INCLUDED__

#define PTRX_CONFIG_FILE    "/usr/local/PeoTrix/conf/peotrix.conf"

#include <log/ptrx_log.h>

char *ptrx_config_get(char *filename, char *item, ptrx_log_t *log);

#endif
