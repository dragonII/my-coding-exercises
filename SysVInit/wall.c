/*
 * wall.c       Write to all users logged in.
 *
 * Usage:       wall [text]
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <syslog.h>

#include "dowall.h"

char *Version = "wall 1-Jul-2013 longwa@cisco.com";
#define MAXLEN  4096
#define MAXLINES  20

int main(int argc, char **argv)
{
    char buf[MAXLEN];
    char line[83];
    int i, f, ch;
    int len = 0;
    int remote = 0;
    char *p;
    char *whoami;
    struct passwd *pwd;

    buf[0] = 0;
    if((pwd = getpwuid(getuid())) == NULL)
    {
        if(getuid() == 0)
            whoami = "root";
        else
        {
            fprintf(stderr, "You don't exist. Go away.\n");
            exit(1);
        }
    } else
        whoami = pwd->pw_name;

    while((ch = getopt(argc, argv, "n")) != EOF)
        switch(ch)
        {
            case 'n':
                /*
                 * Undocumented option for suppressing
                 * banner from rpc.rwalld. Only works if
                 * we are root or if we're NOT setgid.
                 */
                if(geteuid() != 0 && getgid() != getegid())
                {
                    fprintf(stderr, "wall -n: not priviliged\n");
                    exit(1);
                }
                remote = 1;
                break;
            default:
                fprintf(stderr, "usage: wall [message]\n");
                return 1;
                break;
        }

    if((argc - optind) > 0)
    {
        for(f = optind; f < argc; f++)
        {
            len += strlen(argv[f]) + 1;
            if(len >= MAXLEN - 2) break;
            strcat(buf, argv[f]);
            if(f < argc - 1) strcat(buf, " ");
        }
        strcat(buf, "\r\n");
    } else
    {
        while(fgets(line, 80, stdin))
        {
            /*
             * Make sure that line ends in \r\n.
             */
            for(p = line; *p && *p != '\r' && *p != '\n'; p++)
                ;
            strcpy(p, "\r\n");
            len += strlen(line);
            if(len >= MAXLEN) break;
            strcat(buf, line);
        }
    }

    i = 0;
    for(p = buf; *p; p++)
    {
        if(*p == '\n' && i++ > MAXLINES)
        {
            *++p = 0;
            break;
        }
    }

    openlog("wall", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "wall: user %s broadcast %d lines (%d chars)",
                whoami, i, (int)strlen(buf));
    closelog();

    unsetenv("TZ");
    wall(buf, 0, remote);

    return 0;
}
