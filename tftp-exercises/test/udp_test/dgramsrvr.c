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

struct sockaddr_in adr_inet;
struct sockaddr_in adr_clnt;
struct sockaddr_in adr_srvr;

char dgram[512];
char dtfmt[512];


static void bail(const char* on_what)
{
    fputs(strerror(errno), stderr);
    fputs(": ", stderr);
    fputs(on_what, stderr);
    fputc('\n', stderr);
    exit(1);
}

int check_quit()
{
    if(strcasecmp(dgram, "QUIT") == 0)
        return 1;
    else
        return 0;
}

int srvraddr_init(char* srvr_addr)
{
    int s, z;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s == -1)
        bail("socket()");

    memset(&adr_inet, 0, sizeof(adr_inet));
    adr_inet.sin_family = AF_INET;
    adr_inet.sin_port = htons(9090);
    printf("srvr_addr: %s\n", srvr_addr);
    adr_inet.sin_addr.s_addr = inet_addr(srvr_addr);

    if(adr_inet.sin_addr.s_addr == INADDR_NONE)
        bail("bad address.");

    z = bind(s, (struct sockaddr*)&adr_inet, sizeof(adr_inet));
    if(z == -1)
        bail("bind()");

    return s;
}

int new_srvr_addr(char* srvr_addr)
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s == -1)
        bail("socket()");
    
    memset(&adr_srvr, 0, sizeof(adr_srvr));
    adr_srvr.sin_family = AF_INET;
    adr_srvr.sin_port = htons(INADDR_ANY);
    adr_srvr.sin_addr.s_addr = inet_addr(srvr_addr);
    
    printf("new srvr_addr %s:%d\n", 
                inet_ntoa(adr_srvr.sin_addr),
                ntohs(adr_srvr.sin_port));
    return s;
}

void udp_recv(int s)
{
    int len_inet, z, i;

    len_inet = sizeof(adr_clnt);
    z = recvfrom(s, dgram, sizeof(dgram), 0, 
                    (struct sockaddr*)&adr_clnt, (socklen_t*)&len_inet);

    if(z < 0)
        bail("recvfrom(2)");

    printf("[%d]received %d bytes, from %s:%d\n", getpid(), z,
                inet_ntoa(adr_clnt.sin_addr), 
                ntohs(adr_clnt.sin_port));
    for(i = 0; i < z; i++)
        if(dgram[i] == '\r' || dgram[i] == '\n')
            dgram[i] = 0;
    dgram[z] = 0;
    printf("%s received\n", dgram);
}

void udp_send(int s)
{
    int z = sendto(s, dtfmt, strlen(dtfmt), 0, 
                (struct sockaddr*)&adr_clnt,
                sizeof(adr_clnt));
    
    if(z < 0)
        bail("sendto(2)");
    
    printf("[%d]Send %d bytes, to %s:%d\n", getpid(), z,
                inet_ntoa(adr_clnt.sin_addr), 
                ntohs(adr_clnt.sin_port));
}

int main(int argc, char** argv)
{
    int s;
    char* srvr_addr = NULL;
    time_t td;
    struct tm tm;

    pid_t  childpid;

    if(argc >= 2)
        srvr_addr = argv[1];
    else
        srvr_addr = "127.0.0.1";

    s = srvraddr_init(srvr_addr);

    for( ; ; )
    {
        udp_recv(s);

        if(!strcasecmp(dgram, "QUIT"))
            break;

        childpid = fork();
        if(childpid < 0)
            bail("fork error");
        if(childpid > 0)    // parent, keep waiting for new request
        {
            printf("[%d] parent keep waiting\n", getpid());
            continue;
        }
        else
        {
            // Child process
            printf("[%d] child handles the process\n", getpid());
            close(s);       // use a new socket for client
            s = new_srvr_addr(srvr_addr);
            for(;;)
            {
                time(&td);
                tm = *localtime(&td);
                strftime(dtfmt, sizeof(dtfmt), dgram, &tm);

                udp_send(s);

                udp_recv(s);
                if(!strcasecmp(dgram, "QUIT"))
                {
                    printf("[%d] Got quit\n", getpid());
                    exit(1);
                }
            }
        }
    }

    close(s);
    return 0;
}

