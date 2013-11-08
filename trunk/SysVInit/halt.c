/*
 * halt     Stop the system running.
 *      It re-enables CTRL-ALT-DEL, so that a hard reboot can
 *      be done. If called as reboot, it will reboot the system.
 *
 *      If the system is not in runlevel 0 or 6, halt will just
 *      execute a "shutdown -h" to halt the system, and reboot will
 *      execute a "shutdown -r". This is for compatibility with 
 *      sysvinit 2.4.
 *
 * Usage:   halt [-n] [-w] [-d] [-f] [-h] [-i] [-p]
 *      -n: don't sync before halting the system
 *      -w: only write a wtmp reboot record and exit
 *      -d: don't write a wtmp record
 *      -f: force halt/reboot, don't call shutdown
 *      -h: put harddisks in standby mode
 *      -i: shut down all network interfaces
 *      -p: power down the system (if possible, otherwise halt)
 *
 *      Reboot and halt are both this program. Reboot
 *      is just a link to halt. Invoking the program
 *      as poweroff implies the -p option.
 *
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#include "utmp_.h"
#include "reboot_.h"

char *progname;


int main(int argc, char **argv)
{
    int do_reboot = 0;
    int do_sync = 1;
    int do_wtmp = 1;
    int do_nothing = 0;
    int do_hard = 0;
    int do_ifdown = 0;
    int do_hddown = 0;
    int do_poweroff = 0;
    int c;
    char *tm = NULL;

    /* find out who we are */
    if((progname = strrchr(argv[0], '/')) != NULL)
        progname++;
    else
        progname = argv[0];

    if(!strcmp(progname, "reboot")) do_reboot = 1;
    if(!strcmp(progname, "poweroff")) do_poweroff = 1;

    /* get flags */
    while((c = getopt(argc, argv, ":ihdfnpwt:")) != EOF)
    {
        switch(c)
        {
            case 'n':
                do_sync = 0;
                do_wtmp = 0;
                break;
            case 'w':
                do_nothing = 1;
                break;
            case 'd':
                do_wtmp = 0;
                break;
            case 'f':
                do_hard = 1;
                break;
            case 'i':
                do_ifdown = 1;
                break;
            case 'h':
                do_hddown = 1;
                break;
            case 'p':
                do_poweroff = 1;
                break;
            case 't':
                tm = optarg;
                break;
            default:
                usage();
        }
    }

    if(argc != optind) usage();

    if(geteuid() != 0)
    {
        fprintf(stderr, "%s: must be superuser.\n", progname);
        exit(1);
    }

    (void)chdir("/");

    if(!do_hard && !do_nothing)
    {
        /* see if we are in runlevel 0 or 6 */
        c = get_runlevel();
        if(c != '0' && c != '6')
            do_shutdown(do_reboot ? "-r" : "-h", tm);
    }

    /* record the fact that we're going down */
    if(do_wtmp)
        write_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");

    /* exit if all we wanted to do was write a wtmp record */
    if(do_nothing && !do_hddown && !do_ifdown) exit(0);

    if(do_sync)
    {
        sync();
        sleep(2);
    }

    if(do_ifdown)
        (void)ifdown();

    if(do_hddown)
        (void)hddown();

    if(do_nothing) exit(0);

    if(do_reboot)
    {
        init_reboot(BMAGIC_REBOOT);
    } else
    {
        /* turn on hard reboot. CTRL-ALT-DEL will reboot now */
#ifdef BMAGIC_HARD
        init_reboot(BMAGIC_HARD);
#endif
        /* stop init; it is insensitive to the signals sent
         * by the kernel */
        kill(1, SIGTSTP);

        /* halt or poweroff */
        if(do_poweroff)
            init_reboot(BMAGIC_POWEROFF);

        /* fallthrough if failed */
        init_reboot(BMAGIC_HALT);
    }

    /* if we return, we (c)ontinued from the kernel monitor */
#ifdef  BMAGIC_SOFT
    init_reboot(BMAGIC_SOFT);
#endif
    kill(1, SIGCONT);

    exit(0);
}

