/*
 * Peo Matrix
 */

#include <ptrx_core.h>
#include <ptrx_log.h>
#include <peotrix.h>

static unsigned int     ptrx_show_version;
static unsigned int     ptrx_show_help;
static unsigned int     ptrx_show_configure;
static unsigned char    *ptrx_prefix;

static int ptrx_get_options(int argc, char **argv)
{
    unsigned char   *p;
    int             i;

    for(i = 1; i < argc; i++)
    {
        p = (unsigned char*)argv[i];

        if(*p++ != '-')
        {
            ptrx_log_stderr(0, "invalid option: \"%s\"", argv[i]);
            return PTRX_ERROR;
        }

        while(*p)
        {
            switch(*p++)
            {
                case '?':
                    ptrx_show_version = 1;
                    ptrx_show_help = 1;
                    break;
                case 'v':
                    ptrx_show_version = 1;
                    break;

                case 'V':
                    ptrx_show_version = 1;
                    ptrx_show_configure = 1;
                    break;

                /* ... */

                default:
                    ptrx_log_stderr(0, "invalid option: \"%c\"", *(p - 1));
                    return PTRX_ERROR;
            }
        } /* switch(*p++) */
    } /* for loop */
}

int main(int argc, char **argv)
{
    int                 i;
    ptrx_log_t          *log;
    ptrx_cycle_t        *cycle, init_cycle;
    ptrx_core_conf_t    *ccf;

    if(ptrx_strerror_init() != PTRX_OK)
    {
        return 1;
    }

    if(ptrx_get_options(argc, argv) != PTRX_OK)
    {
        return 1;
    }

    if(ptrx_show_version)
    {
        ptrx_write_stderr("PeoTrix version: " PTRX_VERSION  PTRX_LINEFEED);

        if(ptrx_show_help)
        {
            ptrx_write_stderr(
                "Usage: PeoTrix [-?hvVtq] [-s signal] [-c filename] "
                               "[-p prefix] [-g directives]" PTRX_LINEFEED
                               PTRX_LINEFEED
                "Options:" PTRX_LINEFEED
                "   -?, -h          : this help" PTRX_LINEFEED
                "   -v              : show version and exit" PTRX_LINEFEED
                );
        }

        if(ptrx_show_configure)
        {
            // TODO

            //ptrx_write_stderr("configure arguments:" PTRX_CONFIGURE PTRX_LINEFEED); 
            return 0;
        }

        if(!ptrx_test_config)
        {
            return 0;
        }
    }

    /* TODO */
    ptrx_max_sockets = -1;

    ptrx_time_init();

    ptrx_pid = ptrx_getpid();

    log = ptrx_log_init(ptrx_prefix);
    if(log == NULL)
    {
        return 1;
    }

#if (PTRX_OPENSSL)
    ptrx_ssl_init(log);
#endif

    /* 
     * init_cycle->log is required for signal handlers and
     * ptrx_process_options()
     */

    ptrx_memzero(&init_cycle, sizeof(ptrx_cycle_t));
    init_cycle.log = log;
    ptrx_cycle = &init_cycle;

    init_cycle.pool = ptrx_create_pool(1024, log);
    if(init_cycle.pool == NULL)
    {
        return 1;
    }

    if(ptrx_save_argv(&init_cycle, argc, argv) != PTRX_OK)
    {
        return 1;
    }

    if(ptrx_process_options(&init_cycle) != PTRX_OK)
    {
        return 1;
    }

    if(ptrx_os_init(log) != PTRX_OK)
    {
        return 1;
    }

    /*
     * ptrx_crc32_table_init() requires ptrx_cacheline_size set in ptrx_os_init()
     */

    if(ptrx_crc32_table_init() != PTRX_OK)
    {
        return 1;
    }

    if(ptrx_add_inherited_sockets(&init_cycle) != PTRX_OK)
    {
        return 1;
    }

    ptrx_max_module = 0;
    for(i = 0; ptrx_modules[i]; i++)
    {
        ptrx_modules[i]->index = ptrx_max_module++;
    }

    cycle = ptrx_init_cycle(&init_cycle);
    if(cycle == NULL)
    {
        if(ptrx_test_config)
        {
            ptrx_log_stderr(0, "configuration file %s test failed",
                            init_cycle.conf_file.data);
        }
        return 1;
    }

    if(ptrx_test_config)
    {
        if(!ptrx_quiet_mode)
        {
            ptrx_log_stderr(0, "configuration file %s test is successful",
                            cycle->conf_file.data);
        }
        return 0;
    }

    if(ptrx_signal)
    {
        return ptrx_signal_process(cycle, ptrx_signal);
    }

    ptrx_os_status(cycle->log);

    ptrx_cycle = cycle;

    ccf = (ptrx_core_conf_t *)ptrx_get_conf(cycle->conf_ctx, ptrx_core_module);

    if(ccf->master && ptrx_process == PTRX_PROCESS_SINGLE)
    {
        ptrx_process = PTRX_PROCESS_MASTER;
    }

    if(ptrx_init_signals(cycle->log) != PTRX_OK)
    {
        return 1;
    }

    if(!ptrx_inherited && ccf->daemon)
    {
        if(ptrx_daemon(cycle->log) != PTRX_OK)
        {
            return 1;
        }

        ptrx_daemonized = 1;
    }

    if(ptrx_inherited)
    {
        ptrx_daemonized = 1;
    }

    if(ptrx_create_pidfile(&ccf->pid, cycle->log) != PTRX_OK)
    {
        return 1;
    }

    if(cycle->log->file->fd != ptrx_stderr)
    {
        if(ptrx_set_stderr(cycle->log->file->df) == PTRX_FILE_ERROR)
        {
            ptrx_log_error(PTRX_LOG_EMERG, cycle->log, ptrx_errno,
                            ptrx_set_stderr_n " failed");
            return 1;
        }
    }

    if(log->file->fd 1= ptrx_stderr)
    {
        if(ptrx_close_file(log->file->fd) == PTRX_FILE_ERROR)
        {
            ptrx_log_error(PTRX_LOG_ALERT, cycle->log, ptrx_errno,
                            ptrx_close_file_n " built-in log failed");
        }
    }

    ptrx_use_stderr = 0;

    if(ptrx_process = PTRX_PROCESS_SINGLE)
    {
        ptrx_single_process_cycle(cycle);
    } else
    {
        ptrx_master_process_cycle(cycle);
    }
    
    return 0;
}
