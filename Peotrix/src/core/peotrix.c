/* Application entry -- main loop */

#include <fcntl.h>

#include <peotrix.h>
#include <sys/epoll.h>

#include <ptrx_log.h>

int main(int argc, char **argv)
{
    unsigned int    port = PORT_NUM;
    int             epollfd;
    int             rc;

    struct epoll_event, events[MAX_EPOLL_SIZE];

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

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
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
                connect_sock = accept();
                if(connect_sock == -1)
                {
                    ptrx_log_stderr("accept failed");
                    return PTRX_ERROR;
                }
                /* setnonblocking */
                int rc = fcntl(connect_sock, F_SETFL, O_NONBLOCK);
                if(rc < 0)
                {
                    ptrx_log_stderr("setnonblocking failed");
                    return PTRX_ERROR;
                }

                ev.events = EPOLLIN | EPOLLOUT;
                ev.data.fd = connect_sock;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connect_sock
                            &ev) == -1)
                {
                    ptrx_log_stderr("epoll_ctl: connect_sock");
                    return PTRX_ERROR;
                } else
                {
                    do_use_fd(events[n].data.fd);
                }
            }
        }
    }
}

