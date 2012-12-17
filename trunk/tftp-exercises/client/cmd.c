/*
 * Command processing functions, one per command.
 * (Only the client side processes user commands)
 */

#include "cmd.h"
#include "defs_cli.h"
#include "error_cli.h"
#include "cmdsubr.h"
#include "cmdgetput.h"

#include <string.h>
#include <stdlib.h>

/*
 * ascii
 *
 * Equivalent to "mode ascii"
 */

void cmd_ascii()
{
    modetype = MODE_ASCII;
}

/*
 * binary
 *
 * Equivalent to "mode binary"
 */

void cmd_binary()
{
    modetype = MODE_BINARY;
}

/*
 * connect <hostname> [ <port> ]
 *
 * Set the hostname and optional port number for future transfers.
 * The port is the well-know port number of the tftp server on
 * the other system. Normally this will default to the value
 * specified in /etc/services (69).
 */
void cmd_connect()
{
    int val;

    if(gettoken(hostname) == NULL)
        err_cmd("missing hostname");

    if(gettoken(temptoken) == NULL)
        return;

    val = atoi(temptoken);
    if(val < 0)
        err_cmd("invalid port number");

    port = val;
}

void cmd_exit()
{
    D_printf("cmd_exit: exiting...\n");
    exit(0);
}

/*
 * get <remotefilename> <localfilename>
 *
 * Note that the <remotefilename> may be of the form <host>:<filename>
 * to specify the host also.
 */

void cmd_get()
{
    char    remfname[MAXFILENAME], locfname[MAXFILENAME];

    if(gettoken(remfname) == NULL)
        err_cmd("the remote filename must be specified");
    if(gettoken(locfname) == NULL)
        err_cmd("the local filename must be specified");

    if(index(locfname, ':') != NULL)
        err_cmd("cannot have 'host:' in local filename");

    striphost(remfname, hostname);      // check for "host:" and process
    if(hostname[0] == 0)
        err_cmd("no host has been specified");

    do_get(remfname, locfname);
}

void cmd_help()
{
    int i;
    for(i = 0; i < ncmds; i++)
    {
        printf(" %s\n", commands[i].cmd_name);
    }
}


/*
 * mode ascii
 * mode binary
 *
 * Set the mode for file transfers.
 */
void cmd_mode()
{
    if(gettoken(temptoken) == NULL)
        err_cmd("a mode type must be specified");
    else
    {
        if(strcmp(temptoken, "ascii") == 0)
            modetype = MODE_ASCII;
        else if(strcmp(temptoken, "binary") == 0)
            modetype = MODE_BINARY;
        else
            err_cmd("mode must be 'ascii' or 'binary'");
    }
}

/*
 * put <localfilename> <remotefilename>
 *
 * Note that the <remotefilename> may be of the form <host>:<filename>
 * to specify the host also.
 */

void cmd_put()
{
    char remfname[MAXFILENAME], locfname[MAXFILENAME];

    if(gettoken(locfname) == NULL)
        err_cmd("the local filename must be specified");
    if(gettoken(remfname) == NULL)
        err_cmd("the remote filename must be specified");

    if(index(locfname, ':') != NULL)
        err_cmd("cannot have 'host:' in local filename");

    striphost(remfname, hostname);      // check for "host:" and process
    if(hostname[0] == 0)
        err_cmd("no host has been specified");

    do_put(remfname, locfname);
}

/*
 * Show current status.
 */
void cmd_status()
{
    if(connected)
        printf("Connected\n");
    else
        printf("Not connected\n");

    printf("mode = ");
    switch(modetype)
    {
        case MODE_ASCII:  printf("netascii"); break;
        case MODE_BINARY: printf("octet (binary)"); break;
        default:
            err_ret("unknown modetype");
    }

    printf(", verbose = %s\n", verboseflag ? "on" : "off");
}

/*
 * Toggle verbose mode.
 */
void cmd_verbose()
{
    verboseflag = !verboseflag;
}
