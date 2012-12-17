#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char** argv)
{
    char* srvr_ip = NULL;
    unsigned int port = -1;
    struct sockaddr_in srvr_addr;
    int len_inet;
    int sockfd;
    char dgram[512];
    int  data_len;
    int  rc;

    if(argc != 3)
    {
        printf("Usage: %s ip_addr port\n", argv[0]);
        exit(1);
    } else
    {
        srvr_ip = argv[1];
        port = htons(atoi(argv[2]));
    }


    bzero(&srvr_addr, sizeof(srvr_addr));
    
    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_port = port;
    srvr_addr.sin_addr.s_addr = inet_addr(srvr_ip);

    if(srvr_addr.sin_addr.s_addr == INADDR_NONE)
    {
        printf("Invalid address, exit\n");
        exit(1);
    }

    len_inet = sizeof(srvr_addr);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("socket error\n");
        exit(1);
    }

again:
    bzero(dgram, 512);
    printf("\nEnter string: ");
    data_len = scanf("%s", dgram);
    if(data_len > 512)
    {
        printf("too more data, input again:\n");
        goto again;
    }

    int z = strlen(dgram);
    if(z > 0 && dgram[--z] == '\n')
        dgram[z] = 0;

    rc = sendto(sockfd, dgram, strlen(dgram), 0,
                (struct sockaddr*)&srvr_addr, len_inet);

    if(rc < 0)
    {
        printf("sendto error\n");
        exit(1);
    }

    close(sockfd);
    putchar('\n');

    return 0;
}


