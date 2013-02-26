/* Traverse a file hierarchy */

#include "fts_.h"
#include "hash.h"
#include "config.h"
#include "fts-cycle.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


#ifndef __set_errno
# define __set_errno(Val) errno = (Val)
#endif

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define CLR(opt)    (sp->fts_options &= ~(opt))
#define ISSET(opt)  (sp->fts_options & (opt))
#define SET(opt)    (sp->fts_options |= (opt))

/* fts_set takes the stream as an argument alghough it's not used in this
   implementation; it would be necessary if anyone wanted to add global
   semantics to fts using fts_set. An error return is allowed for similar
   reasons */ 
int fts_set(FTS* sp _GL_UNUSED, FTSENT* p, int instr)
{
    if(instr != 0 && instr != FTS_AGAIN && instr != FTS_FOLLOW &&
        instr != FTS_NOINSTR && instr != FTS_SKIP)
    {
        __set_errno(EINVAL);
        return 1;
    }

    p->fts_instr = instr;
    return 0;
}

static size_t
fts_maxarglen(char** argv)
{
    size_t len, max;
    for(max = 0; *argv; ++argv)
        if((len = strlen(*argv)) > max)
            max = len;

    return max + 1;
}

FTS* fts_open(char** argv, int options,
              int (*compar)(FTSENT**, FTSENT**))
{
    FTS* sp;
    FTSENT *p, *root;
    size_t nitems;
    FTSENT* parent = NULL;
    FTSENT* tmp = NULL;         /* pacify gcc */
    bool defer_stat;

    /* Options check */
    if(options & ~FTS_OPTIONMASK)
    {
        __set_errno(EINVAL);
        return NULL;
    }
    if((options & FTS_NOCHDIR) && (options & FTS_CWDFD))
    {
        __set_errno(EINVAL);
        return NULL;
    }
    if(! (options & (FTS_LOGICAL | FTS_PHYSICAL)))
    {
        __set_errno(EINVAL);
        return NULL;
    }

    /* Allocate/initiate the stream */
    if((sp = malloc(sizeof(FTS))) == NULL)
        return NULL;
    memset(sp, 0, sizeof(FTS));
    sp->fts_compar = compar;
    sp->fts_options = options;

    /* Logical walks turn on NOCHDIR; symbolic links are too hard */
    if(ISSET(FTS_LOGICAL))
    {
        SET(FTS_NOCHDIR);
        CLR(FTS_CWDFD);
    }

    /* Initialize fts_cwd_fd */
    sp->fts_cwd_fd = AT_FDCWD;

    /* Start out with 1K of file name space, and enough, in any case,
       to hold the user's file names */
#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif
    {
        size_t maxarglen = fts_maxarglen(argv);
        if(! fts_palloc(sp, MAX(maxarglen, MAXPATHLEN)))
            goto mem1;
    }

    /* Allocate/initialize root's parent */
    if(*argv != NULL)
    {
        if((parent = fts_alloc(sp, "", 0)) == NULL)
            goto mem2;
        parent->fts_level = FTS_ROOTPARENTLEVEL;
    }

    /* The classic fts implementation would call fts_stat with
       a new entry for each iteration of the loop below.
       If the comparison function is not specified or if the
       FTS_DEFER_STAT option is in effect, don't stat any entry
       in this loop. This is an attempt to minimize the interval
       between the initial stat/lstat/fstatat and the point at which
       a directory argument is first opened. This matters for any
       directory command line argument that resides on a file system
       without genuine i-nodes. If you specify FTS_DEFER_STAT along
       with a comparison function, that function must not access any
       data via the fts_statp pointer. */
    defer_stat = (compar == NULL || ISSET(FTS_DEFER_STAT));

    /* Allocate/initialize root(s) */
    for(root = NULL, nitems = 0; *argv != NULL; ++argv, ++nitems)
    {
        /* Do allow zero-length file names */
        size_t len = strlen(*argv);
        if((p = fts_alloc(sp, *argv, len)) == NULL)
            goto mem3;
        p->fts_level = FTS_ROOTLEVEL;
        p->fts_parent = parent;
        p->fts_accpath = p->fts_name;
        /* Even when defer_stat is true, be sure to stat the first
           command line argument, since fts_read (at least with
           FTS_XDEV) requires that */
        if(defer_stat && root != NULL)
        {
            p->fts_info = FTS_NSOK;
            fts_set_stat_required(p, true);
        }
        else
            p->fts_info = fts_stat(sp, p, flase);
    
        /* If comparison routine supplied, traverse in sorted
           order; otherwise traverse in the order specified. */
        if(compar)
        {
            p->fts_link = root;
            root = p;
        }
        else
        {
            p->fts_link = NULL;
            if(root == NULL)
                tmp = root = p;
            else
            {
                tmp->fts_link = p;
                tmp = p;
            }
        }
    }
    if(compar && nitems > 1)
        root = fts_sort(sp, root, nitems);

    /* Allocates a dummy pointer and make fts_read think that we've just
       finish the node before the root(s); set p->fts_info to FTS_INIT
       so that everything about the "current" node is ignored. */
    if((sp->fts_cur = fts_alloc(sp, "", 0)) == NULL)
        goto mem3;
    sp->fts_cur->fts_link = root;
    sp->fts_cur->fts_info = FTS_INIT;
    if(!setup_dir(sp))
        goto mem3;

    /* If using chdir(2), grab a file descriptor pointing to dot to ensure
       that we can get back here; this could be avoided for some file names,
       but almost certainly not worth the effort. Slashes, symbolic links,
       and ".." are all fairly nasty problems. Note, if we can't get the
       descriptor we run anyway, just more slowly. */
    if(!ISSET(FTS_NOCHDIR) && !ISSET(FTS_CWDFD)
        && (sp->fts_rfd = diropen(sp, ".")) < 0)
        SET(FTS_NOCHDIR);

    i_ring_init(&sp->fts_fd_ring, -1);
    return sp;

mem3:
    fts_lfree(root);
    free(parent);
mem2:
    free(sp->fts_path);
mem1:
    free(sp);
    return NULL;
}

static void fts_lfree(register FTSENT* head)
{
    register FTSENT* p;
    /* Free linked list of structures */
    while((p = head))
    {
        head = head->fts_link;
        free(p);
    }
}

static void fd_ring_clear(I_ring* fd_ring)
{
    while(! i_ring_empty(fd_ring))
    {
        int fd = i_ring_pop(fd_ring);
        if(fd >= 0)
            close(fd);
    }
}

int fts_close(FTS* sp)
{
    register FTSENT *freep, *p;
    int saved_errno = 0;

    /* This still works if we haven't read anything -- the dummy structure
       points to the root list, so we step through to the end of the root
       list which has a valid parent pointer. */
    if(sp->fts_cur)
    {
        for(p = sp->fts_cur; p->fts_level >= FTS_ROOTLEVEL;)
        {
            freep = p;
            p = p->fts_link != NULL ? p->fts_link : p->fts_parent;
            free(freep);
        }
        free(p);
    }

    /* Free up child linked list, sort array, file name buffer */
    if(sp->fts_child)
        fts_lfree(sp->fts_child);
    free(sp->fts_array);
    free(sp->fts_path);

    if(ISSET(FTS_CWDFD))
    {
        if(sp->fts_cwd_fd >= 0)
            if(close(sp->fts_cwd_fd))
                saved_errno = errno;
    }
    else if(!ISSET(FTS_NOCHDIR))
    {
        /* Return to original directory, save errno if necessary */
        if(fchdir(sp->fts_rfd))
            saved_errno = errno;

        /* If close fails, record errno only if saved_errno is zero,
           so that we report the probably-more-meaningful fchdir errno. */
        if(close(sp->fts_rfd))
            if(saved_errno == 0)
                saved_errno = errno;
    }

    fd_ring_clear(&sp->fts_fd_ring);

    if(sp->fts_leaf_optimization_works_ht)
        hash_free(sp->fts_leaf_optimization_works_ht);

    free_dir(sp);

    /* Free up the stream pointer */
    free(sp);

    /* Set errno and return */
    if(saved_errno)
    {
        __set_errno(saved_errno);
        return -1;
    }

    return 0;
}
