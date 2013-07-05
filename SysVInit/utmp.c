/* utmp.c       Routines to read/write the utmp and wtmp files.
 *              Basically just wrappers around the library routines.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <utmp.h>
#include <string.h>
#include <sys/time.h>
#include <sys/utsname.h>

#if defined(__GLIBC__)
#   if (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0) && defined(__powerpc__)
#       define HAVE_UPDWTMP 0
#   else
#       define HAVE_UPDWTMP 1
#   endif
#else
#   define HAVE_UPDWTMP 0
#endif

/* Log an event in the wtmp file (reboot, runlevel) */
void write_wtmp(char *user,     /* name of user */
                char *id,       /* inittab ID */
                int  pid,       /* PID of process */
                int  type,      /* TYPE of entry */
                char *line)     /* which line is this */
{
    int fd;
    struct utmp utmp;
    struct utsname uname_buf;

    /* 
     * Try to open the wtmp file. Note that we even try
     * this if we have updwtmp() so we can see if the
     * wtmp file is accessible.
     */
    if((fd = open(WTMP_FILE, O_WRONLY | O_APPEND)) < 0) return;

#ifdef INIT_MAIN
    /* Note if we are going to write a boot record */
    if(type == BOOT_TIME) wrote_wtmp_reboot++;

    /*
     * See if we need to write a reboot record. The reason that
     * we are being so paranoid is that when we first tried to
     * write the reboot record, /var was possible not mounted
     * yet. As soon as we can open WTMP we write a delayed boot record.
     */
    if(wrote_wtmp_reboot == 0 && type != BOOT_TIME)
        write_wtmp("reboot", "~~", 0, BOOT_TIME, "~");
#endif

    /* Zero the fields and enter new fields */
    memset(&utmp, 0, sizeof(utmp));
    gettimeofday(&utmp.ut_tv, NULL);
    utmp.ut_pid = pid;
    utmp.ut_type = type;
    strncpy(utmp.ut_name, user, sizeof(utmp.ut_name));
    strncpy(utmp.ut_id,   id,   sizeof(utmp.ut_id))
    strncpy(utmp.ut_line, line, sizeof(utmp.ut_line));

    /* put the OS version in place of the hostname */
    if(uname(&uname_buf) == 0)
        strncpy(utmp.ut_host, uname_buf.release, sizeof(utmp.ut_host));

#if HAVE_UPDWTMP
    updwtmp(WTMP_FILE, &utmp);
#else
    write(fd, (char *)&utmp, sizeof(utmp));
#endif
    close(fd);
}

