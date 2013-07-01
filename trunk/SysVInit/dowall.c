/*
 * dowall.c     Write to all users on the system.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <utmp.h>
#include <string.h>
#include <setjmp.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <stdlib.h>

static sigjmp_buf jbuf;


/*
 * Alarm handler
 */
static void handler(int arg)
{
    siglongjmp(jbuf, 1);
}


/* Print a text, escape all characters not in Latin-1 */
static void feputs(char *line, FILE *fp)
{
    unsigned char       *p;
    for(p = (unsigned char *)line; *p; p++)
    {
        if(strchr("\t\r\n", *p) ||
            (*p >= 32 && *p <= 127) || (*p >= 160))
        {
            fputc(*p, fp);
        } else
        {
            fprintf(fp, "^%c", (*p & 0x1f) + 'A' - 1);
        }
    }
    fflush(fp);
}


static void getuidtty(char **userp, char **ttyp)
{
    struct passwd       *pwd;
    uid_t               uid;
    char                *tty;
    static char         uidbuf[32];
    static char         ttynm[32];
    static int          init = 0;

    if(!init)
    {
        uid = getuid();
        if((pwd = getpwuid(uid)) != NULL)
        {
            uidbuf[0] = 0;
            strncat(uidbuf, pwd->pw_name, sizeof(uidbuf) - 1);
        } else
        {
            sprintf(uidbuf, uid ? "uid %d" : "root", (int)uid);
        }

        if((tty = ttyname(0)) != NULL)
        {
            if(strncmp(tty, "/dev/", 5) == 0)
                tty += 5;
            sprintf(ttynm, "(%.28s) ", tty);
        } else
            ttynm[0] = 0;
        init++;
    }

    *userp = uidbuf;
    *ttyp = ttynm;
}


/* Check whether given filename looks like tty device */
static int file_isatty(const char *fname)
{
    struct stat     st;
    int             major;

    if(stat(fname, &st) < 0)
        return 0;

    if(st.st_nlink != 1 || !S_ISCHR(st.st_mode))
        return 0;

    /*
     * It would be an impossible task to list all major/minors
     * of tty devices here, so we just exclude the obvious
     * majors of which just opening has side-effects:
     * printers and tapes.
     */
    major = major(st.st_dev);
    if(major == 1 
        || major == 2
        || major == 6
        || major == 9
        || major == 12
        || major == 16
        || major == 21
        || major == 27
        || major == 37
        || major == 96
        || major == 97
        || major == 206
        || major == 230) return 0;

    return 1;
}

/* Wall function */
void wall(char *text, int fromshutdown, int remote)
{
    FILE                *tp;
    struct sigaction    sa;
    struct utmp         *utmp;
    time_t              t;
    char                term[UT_LINESIZE + 6];
    char                line[81];
    char                *date, *p;
    char                *user, *tty;
    int                 fd, flags;

    /* Make sure tp and fd aren't in a register. Some versions
     * of gcc clobber those after longjump
     */
    (void)&tp;
    (void)&fd;

    getuidtty(&user, &tty);

    /* Get the time */
    time(&t);

    date = ctime(&t);
    for(p = date; *p && *p != '\n'; p++) ;

    *p = 0;

    if(remote)
    {
        snprintf(line, sizeof(line),
                "\007\r\nRemote broadcast message (%s):\r\n\r\n",
                date);
    } else
    {
        snprintf(line, sizeof(line),
                "\007\r\nBroadcast message from %s %s(%s):\r\n\r\n",
                user, tty, date);
    }

    /*
     * Fork to avoid us hanging in a write()
     */
    if(fork() != 0)  /* Parent */
        return;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    setutent();

    while((utmp = getutent()) != NULL)
    {
        if(utmp->ut_type != USER_PROCESS ||
            utmp->ut_user[0] == 0) continue;
        if(strncmp(utmp->ut_line, "/dev/", 5) == 0)
        {
            term[0] = 0;
            strncat(term, utmp->ut_line, UT_LINESIZE);
        } else
            snprintf(term, sizeof(term), "/dev/%.*s",
                        UT_LINESIZE, utmp->ut_line);
        if(strstr(term, "/../")) continue;

        fd = -1;
        tp = NULL;

        /* Open it non-delay */
        if(sigsetjmp(jbuf, 1) == 0)
        {
            alarm(2);
            flags = O_WRONLY | O_NDELAY | O_NOCTTY;
            if(file_isatty(term) && (fd = open(term, flags)) >= 0)
            {
                if(isatty(fd) && (tp = fdopen(fd, "w")) != NULL)
                {
                    fputs(line, tp);
                    feputs(text, tp);
                    fflush(tp);
                }
            }
        }
        alarm(0);
        if(fd >= 0) close(fd);
        if(tp != NULL) fclose(tp);
    }
    endutent();

    exit(0);
}
