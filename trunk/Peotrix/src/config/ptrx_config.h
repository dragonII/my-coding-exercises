#ifndef __PTRX_CONFIG_H_INCLUDED__
#define __PTRX_CONFIG_H_INCLUDED__

#define PTRX_CONFIG_FILE    "/usr/local/PeoTrix/conf/peotrix.conf"

#include <log/ptrx_log.h>

typedef char *  ptrx_pair_t;

typedef struct ptrx_config_s
{
    ptrx_pair_t   *config;
} ptrx_config_t;

int ptrx_config_init(ptrx_config_t *conf);

char *ptrx_config_get(char *filename, char *item, ptrx_log_t *log);

#endif
