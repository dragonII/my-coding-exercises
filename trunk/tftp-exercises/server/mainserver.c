/*
 * tftp - Trivial File Transfer Protocol.
 *
 * -p port# specifies a different port# to listen on
 */


#include "defs.h"
#include "daemon.h"
#include "net_udp.h"
#include "rtt.h"
#include "fsm.h"

#include <stdlib.h>


int main(int argc, char** argv)
{
    int childpid;
    char* s;

    D_printf("main: simple tftpd\n");

    while(--argc > 0 && (*++argv)[0] == '-')
        for(s = argv[0] + 1; *s != '\0'; s++)
            switch(*s) {
                case 'p':
                    if(--argc <= 0)
                    {
                        D_printf("main: -p requires another argument\n");
                        exit(1);
                    }
                    port = atoi(*++argv);
                    if(port < 0 || port > 65536)
                    {
                        D_printf("main: Invalid port: %d\n", port);
                        exit(1);
                    }
                    D_printf("main: port: %d\n", port);
                    break;

                default:
                {
                    D_printf("main: unknown command line option: %c\n", *s);
                    exit(1);
                }
            }

    net_init(TFTP_SERVICE, port);

    /*
     * Concurrent server loop.
     * The child created by net_open() handles the client's request.
     * The parent waits for another request. In the inetd case,
     * the parent from net_open() never returns.
     */

    for(; ;)
    {
        if((childpid = net_open(inetdflag)) == 0)
        {
            D_printf("main: child in loop\n");
            fsm_loop(0);    // child process client's request
            net_close();    // then we'are done;
            exit(0);
        }
        // parent waits for another client's request
        D_printf("main: parent is waiting\n");
    }
    // Never reached here
}
