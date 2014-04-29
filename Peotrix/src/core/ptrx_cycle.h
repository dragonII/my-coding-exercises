#ifndef __PTRX_CYCLE_H__
#define __PTRX_CYCLE_H__

#include <ptrx_core.h>
#include <ptrx_palloc.h>
#include <ptrx_connection.h>
#include <ptrx_array.h>
#include <ptrx_list.h>

typedef struct ptrx_cycle_s ptrx_cycle_t;

unsigned int            ptrx_test_config;
volatile ptrx_cycle_t   *ptrx_cycle;

struct ptrx_cycle_s
{
    void                ****conf_ctx;
    ptrx_pool_t         *pool;

    ptrx_log_t          *log;
    ptrx_log_t          new_log;

    ptrx_connection_t   **files;
    ptrx_connection_t   *free_connections;
    unsigned int        free_connection_n;

    ptrx_queue_t        reusable_connections_queue;

    ptrx_array_t        listening;
    ptrx_array_t        pathes;
    ptrx_list_t         open_files;
    ptrx_list_t         shared_memory;

    unsigned int        connection_n;
    unsigned int        files_n;

    ptrx_connection_t   *connections;
    ptrx_event_t        *read_events;
    ptrx_event_t        *write_events;

    ptrx_cycle_t        *old_cycle;

    ptrx_str_t          conf_file;
    ptrx_str_t          conf_param;
    ptrx_str_t          conf_prefix;
    ptrx_str_t          prefix;
    ptrx_str_t          lock_file;
    ptrx_str_t          hostname;
};


typedef struct
{
    int             daemon;
    int             master;

    unsigned int    timer_resolution;

    int             worker_processes;
    int             debug_points;

    int             rlimit_nofile;
    int             rlimit_sigpending;
    off_t           rlimit_core;

    int             priority;

    unsigned int    cpu_affinity_n;
    unsigned long   *cpu_affinity;

    char            *username;
    uid_t           user;
    gid_t           group;

    ptrx_str_t      working_directory;
    ptrx_str_t      lock_file;

    ptrx_str_t      pid;
    ptrx_str_t      oldpid;

    ptrx_array_t    evn;
    char            **environment;

    int             worker_threads;
    size_t          thread_stack_size;
} ptrx_core_conf_t;



#endif
