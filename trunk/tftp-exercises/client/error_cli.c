#include "defs_cli.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char*   pname = NULL;


/*
 * Fatal error. Print a message and terminate.
 */

void err_quit(const char* fmt, ...)
{
    if(pname != NULL)
        D_printf("%s: ", pname);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    exit(1);
}


char* sys_err_str()
{
    static char msgstr[200];
    memset(msgstr, 0, 200);

    if(errno != 0)
        sprintf(msgstr, "(errno = %d)", errno);
    else
        msgstr[0] = '\0';

    return msgstr;
}



/*
 * Fatal error related to a system call. Print a message and terminate.
 * Print the system's errno value and its associated message.
 */

void err_sys(const char* fmt, ...)
{
    if(pname != NULL)
        D_printf("%s: ", pname);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    exit(1);

}



/*
 * Recoverable error. Print a message, and return to caller.
 */

void err_ret(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    perror(sys_err_str());

    fflush(stdout);
    fflush(stderr);

    return;
}

/*
 * User command error.
 * Print out the command line too, for information.
 */
void err_cmd(char* str)
{
    fprintf(stderr, "%s: '%s' command error", pname, command);
    if(strlen(str) > 0)
        fprintf(stderr, ": %s", str);
    fprintf(stderr, "\n");
    fflush(stderr);

    longjmp(jmp_mainloop, 1);   // 1 -> not a timeout, we've already
                                // printed out error message
}
