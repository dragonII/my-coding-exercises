/* Application entry -- main loop */

#include <fcntl.h>

#include <peotrix.h>
#include <sys/epoll.h>

#include <log/ptrx_log.h>
#include <tcp_connection/ptrx_tcp_connection.h>

int main(int argc, char **argv)
{
    unsigned int    port = PORT_NUM;
    int             epollfd;
    int             rc;

    struct epoll_event, events[MAX_EPOLL_SIZE];
    ptrx_tcp_conn_t tcp_serv_info;

    ptrx_log_t  *log;

    rc = ptrx_log_init(log);
    if(rc != PTRX_OK)
    {
        printf("[LOG] Cannot initialize log module, exit\n");
        return PTRX_ERROR;
    }

    rc = ptrx_config_init(config);
    if(rc != PTRX_OK)
    {
        ptrx_log_stderr(log, PTRX_LOG_ERR, 0,
                    "Cannot initialize config module, exit\n");
        return PTRX_ERROR;
    }

    /* create epoll */
    epollfd = epoll_create(MAX_EPOLL_SIZE);

    if(epollfd <= 0)
    {
        ptrx_log_stderr("Cannot create epoll, exit");
        return PTRX_ERROR;
    }

    rc = ptrx_net_init(&tcp_serv_info);
    if(rc != PTRX_OK)
    {
        ptrx_log_stderr("Cannot initialize tcp connectoin, exit");
        return PTRX_ABORT;
    }

    ev.events = EPOLLIN;
    ev.data.fd = tcp_serv_info.sockfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, tcp_serv_info.sockfd, &ev) == -1)
    {
        ptrx_log_stderr("epoll_ctl failed: listen_sock");
        return PTRX_ERROR;
    }

    for(;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EPOLL_SIZE, -1)
        if(nfds == -1)
        {
            ptrx_log_stderr("epoll_wait failed");
            return PTRX_ERROR;
        }

        for(n = 0; n < nfds; n++)
        {
            if(events[n].data.fd == listen_sock)
            {
                rc = ptrx_net_open(&tcp_serv_info); 
                if(rc < 0)
                {
                    ptrx_log_stderr("ptrx_net_open failed");
                    return PTRX_ERROR;
                }
                if(rc == PTRX_OK)
                {   /* parent, keep waiting for next */
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = tcp_serv_info.sockfd;
                    if(epoll_ctl(epollfd, EPOLL_CTL_ADD,tcp_serv_info.sockfd; 
                                &ev) == -1)
                    {
                        ptrx_log_stderr("epoll_ctl: connect_sock");
                        return PTRX_ERROR;
                    } 
                }
                if(rc > 0) /* newsockfd returned */
                {
                    /* TODO: to be finished */
                    process(rc);
                }
            } else
            {
                /* other sockets */
                do_use_fd(events[n].data.fd);
            }
        }
    }
}

