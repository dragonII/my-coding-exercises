#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <locale.h>

#include <stdio.h>
#include <getopt.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/prctl.h>

#include "base.h"

static int l_issetugid(void) {
    return (geteuid() != getuid() || getegid() != getgid());
}

#define issetugid l_issetugid

static volatile sig_atomic_t srv_shutdown = 0;
static volatile sig_atomic_t graceful_shutdown = 0;
static volatile sig_atomic_t handle_sig_alarm = 1;
static volatile sig_atomic_t handle_sig_hup = 0;
static volatile sig_atomic_t forwarded_sig_hup = 0;


int main(int argc, char **argv)
{
    server *srv = NULL;
    int print_config = 0;
    int test_config = 0;
    int i_am_root;
    int o;
    int num_childs = 0;
    int pid_fd = -1, fd;
    size_t i;

    struct sigaction act;

    struct rlimit rlim;

    /* for nice %b handling in strftime() */
    setlocale(LC_TIME, "C");

    if((srv = server_init()) == NULL)
    {
        fprintf(stderr, "did this really happen?\n");
        return -1;
    }

    /* init structs done */

    srv->srvconf.port = 0;

    i_am_root = (getuid() == 0);

    srv->srvconf.dont_daemonize = 0;

    while((o = getopt(argc, argv, "f:m:hvVDpt")) != -1)
    {
        switch(o)
        {
            case 'f':
                if(srv->config_storage)
                {
                    log_error_write(srv, __FILE__, __LINE__, "s",
                        "Can only read one config file. Use the include command to use multiple config files.");
                    server_free(srv);
                    return -1;
                }
                break;
            case 'm':
                buffer_copy_string(srv->srvconf.modules_dir, optarg);
                break;
            case 'p': print_config = 1; break;
            case 't': test_config = 1; break;
            case 'D': srv->srvconf.dont_daemonize = 1; break;
            case 'v': show_version(); return 0;
            case 'V': show_features(); return 0;
            case 'h': show_help(); return 0;
            default:
                show_help();
                server_free(srv);
                return -1;
        }
    }

    if(!srv->config_storage)
    {
        log_error_write(srv, __FILE__, __LINE__, "s",
            "No configuration available. Try using -f option.");
        server_free(srv);
        return -1;
    }

    if(print_config)
    {
        data_unset *dc = srv->config_context->data[0];
        if(dc)
        {
            dc->print(dc, 0);
            fprintf(stdout, "\n");
        } else {
            /* shouldn't happened */
            fprintf(stderr, "global config not found\n");
        }
    }

    if(test_config)
    {
        printf("Syntax OK\n");
    }

    if(test_config || print_config)
    {
        server_free(srv);
        return 0;
    }

    /* close stdin and stdout, as they are not needed */
    openDevNull(STDIN_FILENO);
    openDevNull(STDOUT_FILENO);

    if(config_set_defaults(srv) != 0)
    {
        log_error_write(srv, __FILE__, __LINE__, "s",
            "setting default values failed");
        server_free(srv);
        return -1;
    }

    /* UID handling */
    if(!i_am_root && issetugid())
    {
        /* we are setuid-root */
        log_error_write(srv, __FILE__, __LINE__, "s",
            "Are you nuts ? Don't apply a SUID bit to this binary");
        server_free(srv);
        return -1;
    }

    /* check document root */
    if(srv->config_storage[0]->document_root->used <= 1)
    {
        log_error_write(srv, __FILE__, __LINE__, "s",
            "document-root is not set\n");
        server_free(srv);
        return -1;
    }

    if(plugins_load(srv))
    {
        log_error_write(srv, __FILE__, __LINE__, "s",
            "loading plugins finally failed");
        plugins_free(srv);
        server_free(srv);

        return -1;
    }

    /* open pid file BEFORE chroot */
    if(srv->srvconf.pid_file->used)
    {
        if((pid_fd = open(srv->srvconf.pid_file->ptr,
                            O_WRONLY | O_CREAT | O_EXCL | O_TRUNC,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
        {
            struct stat st;
            if(errno != EEXIST)
            {
                log_error_write(srv, __FILE__, __LINE__, "sbs",
                    "opening pid-file failed:", srv->srvconf.pid_file, strerror(errno));
                return -1;
            }

            if(stat(srv->srvconf.pid_file->ptr, &st) != 0)
            {
                log_error_write(srv, __FILE__, __LINE__, "sbs",
                    "stating existing pid-file failed:", srv->srvconf.pid_file, strerror(errno));
            }


