/* shutdown.c   Shut the system down.
 *
 * Usage:   shutdown [-krhfnc] time [warning message]
 *          -k: don't really shutdown, only warn
 *          -r: reboot after shutdown.
 *          -h: halt after shutdonw
 *          -f: do a 'fast' reboot (skip fsck)
 *          -F: Force fsck on reboot.
 *          -n: do not go through init but do it ourselves.
 *          -c: cancel an already running shutdown
 *          -t secs: delay between SIGTERM and SIGKILL for init
 */

 char *Version = "shutdown for my test, 1-Jul-2013 longwa@cisco.com";

#define MESSAGELEN  256

char *clean_env[] = 
{
    "HOME=/",
    "PATH=/bin:/usr/bin:/sbin:/usr/sbin",
    "TERM=dumb",
    NULL,
};


