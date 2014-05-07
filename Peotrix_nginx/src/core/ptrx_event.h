#ifndef __PTRX_EVENT_H__
#define __PTRX_EVENT_H__

#include <ptrx_rbtree.h>

typedef struct ptrx_event_s ptrx_event_t;
typedef void (*ptrx_event_handler_pt)(ptrx_event_t *ev);
struct ptrx_event_s
{
    void                    *data;

    unsigned                write:1;

    unsigned                accept:1;

    /* used to detect the stale events in kqueue, rtsig and epoll */
    unsigned                instance:1;

    /*
     * the event was passed or would be passed to a kernel;
     * in aio mode - operation was posted.
     */
    unsigned                active:1;
    unsigned                disabled:1;

    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned                ready:1;

    unsigned                oneshot:1;

    /* aio operation is complete */
    unsigned                complete:1;

    unsigned                eof:1;
    unsigned                error:1;

    unsigned                timedout:1;
    unsigned                timer_set:1;

    unsigned                delayed:1;

    unsigned                read_discarded:1;

    unsigned                unexpected_eof:1;

    unsigned                deferred_accepted:1;

    /* the pending eof reported by kqueue in aio chain operation */
    unsigned                pending_eof:1;

    ptrx_event_handler_pt   hander;

    unsigned int            index;

    ptrx_log_t              *log;

    ptrx_rbtree_node_t      timer;

    unsigned                closed:1;

    /* to test on worker exit */
    unsigned                channel:1;
    unsigned                resolver:1;

    unsigned                locked:1;

    unsigned                posted_ready:1;
    unsigned                posted_timedout:1;
    unsigned                posted_eof:1;

    //ptrx_atomic_t           *lock;
    //ptrx_atomic_t           *own_lock;

    /* the links of the posted queue */
    ptrx_event_t            *next;
    ptrx_event_t            **prev;
};


#endif
