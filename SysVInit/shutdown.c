/* shutdown.c   Shut the system down.
 *
 * Usage:   shutdown [-krhfnc] time [warning message]
 *          -k: don't really shutdown, only warn
 *          -r: reboot after shutdown.
 *          -h: halt after shutdown
 *          -f: do a 'fast' reboot (skip fsck)
 *          -F: Force fsck on reboot.
 *          -n: do not go through init but do it ourselves.
 *          -c: cancel an already running shutdown
 *          -t secs: delay between SIGTERM and SIGKILL for init
 */

#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <wait.h>
#include <unistd.h>

#include "dowall.h"
#include "initreq.h"
#include "paths.h"

char *Version = "shutdown for my test, 1-Jul-2013 longwa@cisco.com";

#define MESSAGELEN  256

int dontshut = 0;       /* don't shutdown, only warn */
char down_level[2];     /* what runlevel to go to */
int dosync = 1;         /* sync before reboot or halt */
int fastboot = 0;       /* do a 'fast' reboot */
int forcefsck = 0;      /* force a fsck on reboot */
char message[MESSAGELEN];   /* warning message */
char *sltime = 0;       /* sleep time */
char newstate[64];      /* what are we gonna do */
int doself = 0;         /* don't use init */
int got_alrm = 0;

char *clean_env[] = 
{
    "HOME=/",
    "PATH=/bin:/usr/bin:/sbin:/usr/sbin",
    "TERM=dumb",
    NULL,
};

/* Sleep without being interrupted. */
void hardsleep(int secs)
{
    struct timespec ts, rem;

    ts.tv_sec = secs;
    ts.tv_nsec = 0;

    while(nanosleep(&ts, &rem) < 0 && errno == EINTR)
        ts = rem;
}

/* Break off an already running shutdown */
void stopit(int sig)
{
    unlink(NOLOGIN);
    unlink(FASTBOOT);
    unlink(FORCEFSCK);
    unlink(SDPID);
    printf("\r\nShutdown cancelled.\r\n");
    exit(0);
}

/* Show usage message */
void usage()
{
    fprintf(stderr,
    "Usage:\t   shutdown [-akrhHPfnc] [-t secs] [warning message]\n"
    "\t\t   -a:     use /etc/shutdown.allow\n"
    "\t\t   -k:     don't really shutdown, only warn.\n"
    "\t\t   -r:     reboot after shutdown.\n"
    "\t\t   -h:     halt after shutdown.\n"
    "\t\t   -P:     halt action is to turn off power.\n"
    "\t\t   -H:     halt action is to just halt.\n"
    "\t\t   -f:     do a 'fast' reboot (skip fsck).\n"
    "\t\t   -F:     Force fsck on reboot.\n"
    "\t\t   -n:     do not go through \"init\" but go down real fast.\n"
    "\t\t   -c:     cancel a running shutdown.\n"
    "\t\t   -t secs:delay between warning and kill signal.\n"
    "\t\t   ** the \"time\" argument is mandatory! (try \"now\") **\n");
    exit(1);
}

void alrm_handler(int sig)
{
    got_alrm = sig;
}

/* Set environment variables in the init process */
int init_setenv(char *name, char *value)
{
    struct init_request request;
    struct sigaction    sa;
    int                 fd;
    int                 nl, vl;

    memset(&request, 0, sizeof(request));
    request.magic = INIT_MAGIC;
    request.cmd   = INIT_CMD_SETENV;
    nl = strlen(name);
    vl = value ? strlen(value) : 0;

    if(nl + vl + 3 >= sizeof(request.i.data))
        return -1;

    memcpy(request.i.data, name, nl);
    if(value)
    {
        request.i.data[nl] = '=';
        memcpy(request.i.data + nl + 1, value, vl);
    }

    /*
     * Open the fifo and write the command.
     * Make sure we don't hang on opening /dev/initctl
     */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alrm_handler;
    sigaction(SIGALRM, &sa, NULL);
    got_alrm = 0;
    alarm(3);
    if((fd = open(INIT_FIFO, O_WRONLY)) >= 0 &&
            write(fd, &request, sizeof(request)) == sizeof(request))
    {
        close(fd);
        alarm(0);
        return 0;
    }

    fprintf(stderr, "shutdown: ");
    if(got_alrm)
    {
        fprintf(stderr, "timeout opening/writing control channel %s\n",
                    INIT_FIFO);
    } else
    {
        perror(INIT_FIFO);
    }
    return -1;
}


/* Tell everyone the systems is going down in 'mins' minutes. */
void warn(int mins)
{
    char buf[MESSAGELEN + sizeof(newstate)];
    int len;

    buf[0] = 0;
    strncat(buf, message, sizeof(buf) - 1);
    len = strlen(buf);

    if(mins == 0)
        snprintf(buf + len, sizeof(buf) - len,
                "\rThe system is going down %s NOW!\r\n",
                newstate);
    else
        snprintf(buf + len, sizeof(buf) - len,
                "\rThe system is going DOWN %s in %d minute%s!\r\n",
                newstate, mins, mins == 1 ? "" : "s");
    wall(buf, 1, 0);
}

/* create the /etc/nologin file */
void donologin(int min)
{
    FILE *fp;
    time_t t;

    time(&t);
    t += 60 * min;

    if((fp = fopen(NOLOGIN, "w")) != NULL)
    {
        fprintf(fp, "\rThe sytem is going down on %s\r\n", ctime(&t));
        if(message[0]) fputs(message, fp);
        fclose(fp);
    }
}

/* Spawn an external program */
int spawn(int noerr, char *prog, ...)
{
    va_list ap;
    pid_t   pid, rc;
    int i;
    char  *argv[8];

    i = 0;
    while((pid = fork()) < 0 && i < 10)
    {
        perror("fork");
        sleep(5);
        i++;
    }

    if(pid < 0) return -1;

    if(pid > 0) /* parent */
    {
        while((rc = wait(&i)) != pid)
            if(rc < 0 && errno == ECHILD)
                break;
        return (rc == pid) ? WEXITSTATUS(i) : -1;
    }

    if(noerr) fclose(stderr);

    argv[0] = prog;
    va_start(ap, prog);
    for(i = 1; i < 7 && (argv[i] = va_arg(ap, char *)) != NULL; i++)
        ;

    argv[i] = NULL;
    va_end(ap);

    chdir("/");
    environ = clean_env;

    execvp(argv[0], argv);
    perror(argv[0]);
    exit(1);

    /* NOTREACHED */
    return 0;
}


/* Kill all processes, call /etc/init.d/halt (if present) */
void fastdown()
{
    int do_halt = (down_level[0] == '0');
    int i;

    /* first close all files */
    for(i = 0; i < 3; i++)
        if(!isatty(i))
        {
            close(i);
            open("/dev/null", O_RDWR);
        }

    for(i = 3; i < 20; i++) close(i);
    close(255);

    /* first idle init */
    if(kill(1, SIGTSTP) < 0) /* keyboard stop */
    {
        fprintf(stderr, "shutdown: can't idle init.\r\n");
        exit(1);
    }

    /* kill all processes */
    fprintf(stderr, "shutdown: sending all processes the TERM signal...\r\n");
    kill(-1, SIGTERM);
    sleep(sltime ? atoi(sltime) : 3);
    fprintf(stderr, "shutdown: sending all processes the KILL signal.\r\n");
    (void)kill(-1, SIGKILL);

    /* script failed or not present: do it ourself */
    sleep(1);   /* give init the chance to collect zombies */

    /* record the fact that we're going down */
    write_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");

    /* This is for those who have quota installed */
    spawn(1, "accton", NULL);
    spawn(1, "quotaoff", "-a", NULL);

    sync();
    fprintf(stderr, "shutdown: turning off swap\r\n");
    spawn(0, "swapoff", "-a", NULL);
    fprintf(stderr, "shutdown: unmounting all file systems\r\n");
    spawn(0, "umount", "-a", NULL);

    /* we're done, halt or reboot now */
    if(do_halt)
    {
        fprintf(stderr, "The system is halted. Press CTRL-ALT-DEL "
                        "or turn off power\r\n");
        init_reboot(BMAGIC_HALT);
        exit(0);
    }

    fprintf(stderr, "Please stand by while rebooting the system.\r\n");
    init_reboot(BMAGIC_REBOOT);
    exit(0);
}
