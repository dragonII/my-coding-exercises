#ifndef __PTRX_CONF_H__
#define __PTRX_CONF_H__

#include <ptrx_string.h>
#include <ptrx_cycle.h>


//typedef struct ptrx_open_file_s ptrx_open_file_t;
//struct ptrx_open_file_s
//{
//    int             fd;
//    ptrx_str_t      name;
//    unsigned char   *buffer;
//    unsigned char   *pos;
//    unsigned char   *last;
//};

#define PTRX_MODULE_V1          0, 0, 0, 0, 0, 0, 1
#define PTRX_MODULE_V1_PADDING  0, 0, 0, 0, 0, 0, 0, 0

#define PTRX_CONF_UNSET         -1
#define PTRX_CONF_UNSER_UINT    (unsigned int) -1
#define PTRX_CONF_UNSET_PTR     (void *) -1
#define PTRX_CONF_UNSET_SIZE    (size_t) -1
#define PTRX_CONF_UNSET_MSEC    (ptrx_msec_t) -1

#define PTRX_CONF_OK            NULL
#define PTRX_CONF_ERROR         (void *) -1

#define PTRX_CONF_NOARGS      0x00000001
#define PTRX_CONF_TAKE1       0x00000002
#define PTRX_CONF_TAKE2       0x00000004
#define PTRX_CONF_TAKE3       0x00000008
#define PTRX_CONF_TAKE4       0x00000010
#define PTRX_CONF_TAKE5       0x00000020
#define PTRX_CONF_TAKE6       0x00000040
#define PTRX_CONF_TAKE7       0x00000080


#define PTRX_CONF_ARGS_NUMBER 0x000000ff
#define PTRX_CONF_BLOCK       0x00000100
#define PTRX_CONF_FLAG        0x00000200
#define PTRX_CONF_ANY         0x00000400
#define PTRX_CONF_1MORE       0x00000800
#define PTRX_CONF_2MORE       0x00001000
#define PTRX_CONF_MULTI       0x00002000


#define PTRX_DIRECT_CONF        0x00010000
#define PTRX_MAIN_CONF          0x01000000



#define PTRX_CORE_MODULE        0x45524F43 /* "CORE" */

#define ptrx_null_command { ptrx_null_string, 0, NULL, 0, 0, NULL }

typedef struct ptrx_command_s   ptrx_command_t;
typedef struct ptrx_module_s    ptrx_module_t;
typedef struct ptrx_conf_s      ptrx_conf_t;

typedef struct ptrx_conf_file_s
{
    ptrx_file_t     file;
    ptrx_buf_t      *buffer;
    unsigned int    line;
} ptrx_conf_file_t;

typedef struct
{
    ptrx_str_t          name;
    void                *(*create_conf)(ptrx_cycle_t *cycle);
    char                *(*init_conf)(ptrx_cycle_t *cycle, void *conf);
} ptrx_core_module_t;

typedef char *(*ptrx_conf_handler_pt)(ptrx_conf_t *cf,
            ptrx_command_t *dummy, void *conf);

struct ptrx_conf_s
{
    char                    *name;
    ptrx_array_t            *args;

    ptrx_cycle_t            *cycle;
    ptrx_pool_t             *pool;
    ptrx_pool_t             *temp_pool;
    ptrx_conf_file_t        *conf_file;
    ptrx_log_t              *log;

    void                    *ctx;
    unsigned int            module_type;
    unsigned int            cmd_type;

    ptrx_conf_handler_pt    handler;
    char                    *handler_conf;
};

struct ptrx_command_s
{
    ptrx_str_t      name;
    unsigned int    type;
    char            *(*set)(ptrx_conf_t *cf, ptrx_command_t *cmd, void *conf);
    unsigned int    conf;
    unsigned int    offset;
    void            *post;
};


struct ptrx_module_s
{
    unsigned int        ctx_index;
    unsigned int        index;

    unsigned int        spare0;
    unsigned int        spare1;
    unsigned int        spare2;
    unsigned int        spare3;

    unsigned int        version;

    void                *ctx;
    ptrx_command_t      *commands;
    unsigned int        type;

    int                 (*init_master)(ptrx_log_t *log);

    int                 (*init_module)(ptrx_cycle_t *cycle);

    int                 (*init_process)(ptrx_cycle_t *cycle);
    int                 (*init_thread)(ptrx_cycle_t *cycle);
    void                (*exit_thread)(ptrx_cycle_t *cycle);
    void                (*exit_process)(ptrx_cycle_t *cycle);

    void                (*exit_master)(ptrx_cycle_t *cycle);

    int                 spare_hook0;
    int                 spare_hook1;
    int                 spare_hook2;
    int                 spare_hook3;
    int                 spare_hook4;
    int                 spare_hook5;
    int                 spare_hook6;
    int                 spare_hook7;
};

ptrx_module_t   *ptrx_modules[];

int ptrx_conf_full_name(ptrx_cycle_t *cycle,
                        ptrx_str_t   *name,
                        unsigned int  conf_prefix);

char *ptrx_conf_set_flag_slot(ptrx_conf_t *cf, ptrx_command_t *cmd,
                              void *conf);


#define ptrx_get_conf(conf_ctx, module) conf_ctx[module.index]

#define ptrx_conf_init_value(conf, default)     \
    if(conf == PTRX_CONF_UNSET) {               \
        conf = default;                         \
    }

#define ptrx_conf_init_msec_value(conf, default)     \
    if(conf == PTRX_CONF_UNSET_MSEC) {               \
        conf = default;                              \
    }

#define ptrx_conf_init_size_value(conf, default)    \
    if(conf == PTRX_CONF_UNSET_SIZE) {              \
        conf = default;                             \
    }


#endif
