#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void bail(const char* on_what)
{
    fputs(strerror(errno), stderr);
    fputs(": ", stderr);
    fputs(on_what, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char** argv)
{
    int z;
    int x;
    char *srvr_addr = NULL;
    struct sockaddr_in addr_srvr;
    struct sockaddr_in adr;
    int len_inet;
    int s;
    char dgram[512];
    int first_send = 1;

    if(argc >= 2)
        srvr_addr = argv[1];
    else
        srvr_addr = "127.0.0.1";

    memset(&addr_srvr, 0, sizeof(addr_srvr));
    addr_srvr.sin_family = AF_INET;
    addr_srvr.sin_port   = htons(9090);
    addr_srvr.sin_addr.s_addr = inet_addr(srvr_addr);

    len_inet = sizeof(addr_srvr);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s == -1)
        bail("socket()");

    for( ; ; )
    {
        fputs("\nEnter format string: ", stdout);
        if(!fgets(dgram, sizeof(dgram), stdin))
            break;  // EOF

        z = strlen(dgram);
        if(z > 0 && dgram[--z] == '\n')
            dgram[z] = 0;

        if(first_send == 1)
        {
            z = sendto(s, dgram, strlen(dgram), 0,
                        (struct sockaddr*)&addr_srvr, sizeof(addr_srvr));
            first_send = 0;
        } else
        {
            z = sendto(s, dgram, strlen(dgram), 0,
                        (struct sockaddr*)&adr, sizeof(adr));
        }


        if(z < 0)
            bail("sendto(2)");

        printf("Send %d bytes to %s:%d\n", z,
                    inet_ntoa(addr_srvr.sin_addr),
                    ntohs(addr_srvr.sin_port));


        if(!strcasecmp(dgram, "QUIT"))
            break;

        x = sizeof(adr);
        z = recvfrom(s, dgram, sizeof(dgram), 0,
                    (struct sockaddr*)&adr, (socklen_t*)&x);
        if(z < 0)
            bail("recvfrom(2)");

        dgram[z] = 0;

        printf("Result from %s port %u : \n\t'%s'\n",
                    inet_ntoa(adr.sin_addr),
                    (unsigned)ntohs(adr.sin_port),
                    dgram);
    }
    close(s);
    putchar('\n');

    return 0;
}
