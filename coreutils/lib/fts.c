/* Traverse a file hierarchy */

#include "fts_.h"
#include "hash.h"
#include "config.h"
#include "fts-cycle.h"
#include "closexec.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>


#ifndef __set_errno
# define __set_errno(Val) errno = (Val)
#endif

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

bool fts_debug = false;
#define Dprintf(x) do { if (fts_debug) printf x; } while(false)

#define fts_assert(expr)        \
    do                          \
    {                           \
        if(!(expr))             \
            abort();            \
    }                           \
    while(false)                

#define ISDOT(a)    (a[0] == '.' && (!a[1] || (a[1] == '.' && !a[2])))
#define STREQ(a, b) (strcmp((a), (b)) == 0)


#define CLR(opt)    (sp->fts_options &= ~(opt))
#define ISSET(opt)  (sp->fts_options & (opt))
#define SET(opt)    (sp->fts_options |= (opt))

enum Fts_stat
{
    FTS_NO_STAT_REQUIRED = 1,
    FTS_STAT_REQUIRED = 2
};


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

/* Allow essentially unlimited file name length; find, rm, ls should
   all work on any tree. Most systems will allow creation of file
   names much longer than MAXPATHLEN, even though the kernel won't
   resolve them. Add the size (not just what's needed) plus 256 bytes
   so don't realloc the file name 2 bytes at a time. */
static bool fts_palloc(FTS* sp, size_t more)
{
    char* p;
    size_t new_len = sp->fts_pathlen + more + 256;

    /* See if fts_pathlen would overflow */
    if(new_len < sp->fts_pathlen)
    {
        free(sp->fts_path);
        sp->fts_path = NULL;
        __set_errno(ENAMETOOLONG);
        return false;
    }
    sp->fts_pathlen = new_len;
    p = realloc(sp->fts_path, sp->fts_pathlen);
    if(p == NULL)
    {
        free(sp->fts_path);
        sp->fts_path = NULL;
        return false;
    }
    sp->fts_path = p;
    return true;
}


static FTSENT* fts_alloc(FTS* sp, char* name, register size_t namelen)
{
    register FTSENT* p;
    size_t len;

    /* The file name is a variable length array. Allocate the FTSENT
       structure and the file name in one chunk. */
    len = sizeof(FTSENT) + namelen;
    if((p = malloc(len)) == NULL)
        return NULL;

    /* Copy the name and guarantee NUL termination */
    memmove(p->fts_name, name, namelen);
    p->fts_name[namelen] = '\0';

    p->fts_namelen = namelen;
    p->fts_fts = sp;
    p->fts_path = sp->fts_path;
    p->fts_errno = 0;
    p->fts_flags = 0;
    p->fts_instr = FTS_NOINSTR;
    p->fts_number = 0;
    p->fts_pointer = NULL;
    
    return p;
}

/* Overflow the fts_statp->st_size member (otherwise unused, when
   fts_info is FTS_NSOK) to indicate whether fts_read should stat
   this entry or not */
static void fts_set_stat_required(FTSENT* p, bool required)
{
    fts_assert(p->fts_info == FTS_NSOK);
    p->fts_statp->st_size = (required 
                             ? FTS_STAT_REQUIRED
                             : FTS_NO_STAT_REQUIRED);
}


static unsigned short int
fts_stat(FTS* sp, register FTSENT* p, bool follow)
{
    struct stat* sbp = p->fts_statp;
    int saved_errno;

    if(p->fts_level == FTS_ROOTLEVEL && ISSET(FTS_COMFOLLOW))
        follow = true;

    /* If doing a logical walk, or application requested FTS_FOLLOW, do
       a stat(2). If that fails, check for a non-existent symlink. If
       fail, set the errno from the stat call. */
    if(ISSET(FTS_LOGICAL) || follow)
    {
        if(stat(p->fts_accpath, sbp))
        {
            saved_errno = errno;
            if(errno == ENOENT
                && lstat(p->fts_accpath, sbp) == 0)
            {
                __set_errno(0);
                return FTS_SLNONE;
            }
            p->fts_errno = saved_errno;
            goto err;
        }
    }
    else if(fstatat(sp->fts_cwd_fd, p->fts_accpath, sbp,
                        AT_SYMLINK_NOFOLLOW))
    {
        p->fts_errno = errno;
err:        
        memset(sbp, 0, sizeof(struct stat));
        return FTS_NS;
    }
    if(S_ISDIR(sbp->st_mode))
    {
        p->fts_n_dirs_remaining = (sbp->st_nlink - (ISSET(FTS_SEEDOT) ? 0 : 2));
        if(ISDOT(p->fts_name))
        {
            /* Command-line "." and ".." are real directories */
            return (p->fts_level == FTS_ROOTLEVEL ? FTS_D : FTS_DOT);
        }

        return FTS_D;
    }
    if(S_ISLNK(sbp->st_mode))
        return FTS_SL;
    if(S_ISREG(sbp->st_mode))
        return FTS_F;
    return FTS_DEFAULT;
}


static int fts_compar(const void* a, const void* b)
{
    /* Convert A and B to the correct types, to pacify the compiler, and
       for portability to bizarre hosts where "void*" and "FTSENT**" differ
       in runtime representation. The comparison function cannot modify
       *a and *b, but there is no compile-time check for this */
    FTSENT **pa = (FTSENT**)a;
    FTSENT **pb = (FTSENT**)b;
    return pa[0]->fts_fts->fts_compar(pa, pb);
}

static FTSENT* fts_sort(FTS* sp, FTSENT* head, register size_t nitems)
{
    register FTSENT **ap, *p;

    /* On most modern hosts, void* and FTSENT** have the same
       run-time representation, and one can convert sp->fts_compar to
       the type qsort expects without problem. Use the heuristic that
       this is OK if the two pointer types are the same size, and if
       converting FTSENT** to long int is the same as converting
       FTSENT** to void* and then to long int. This heuristic isn't
       valid in general but we don't know of any counterexamples */
    FTSENT* dummy;
    int (*compare) (const void*, const void *) = 
        ((sizeof &dummy == sizeof(void*)
         && (long int) &dummy == (long int)(void*) &dummy)
        ? (int (*) (const void*, const void*))sp->fts_compar
        : fts_compar);

    /* Construct an array of pointers to the structures and call qsort(3).
       Reassemble the array in the order returned by qsort. If unable to
       sort for memory reasons, return the directory entries in their
       current order. Allocate enough space for the current needs plus
       40 so don't realloc one entry at a time. */
    if(nitems > sp->fts_nitems)
    {
        FTSENT **a;

        sp->fts_nitems = nitems + 40;
        if(SIZE_MAX / sizeof *a < sp->fts_nitems
            || !(a = realloc(sp->fts_array,
                             sp->fts_nitems * sizeof *a)))
        {
            free(sp->fts_array);
            sp->fts_array = NULL;
            sp->fts_nitems = 0;
            return head;
        }
        sp->fts_array = a;
    }
    for(ap = sp->fts_array, p = head; p; p = p->fts_link)
        *ap++ = p;
    qsort((void*)sp->fts_array, nitems, sizeof(FTSENT*), compare);
    for(head = *(ap = sp->fts_array); --nitems; ++ap)
        ap[0]->fts_link = ap[1];
    ap[0]->fts_link = NULL;
    return head;
}


/* Open the directory DIR is possible, and return a file
   descriptor. Return -1 and set errno on failure. It doesn't matter
   whether the file descriptor has read or write access */
static inline int
diropen(FTS* sp, char* dir)
{
    int open_flags = (O_RDONLY | O_DIRECTORY | O_NOCTTY | O_NONBLOCK
                      | (ISSET(FTS_PHYSICAL) ? O_NOFOLLOW : 0));
    int fd = (ISSET(FTS_CWDFD)
              ? openat(sp->fts_cwd_fd, dir, open_flags)
              : open(dir, open_flags));
    if(fd >= 0)
        set_closexec_flag(fd, true);
    return fd;
}


static void fts_lfree(register FTSENT* head)
{
    register FTSENT* p;
    /* Free a linked list of structures */
    while((p = head))
    {
        head = head->fts_link;
        free(p);
    }
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
            p->fts_info = fts_stat(sp, p, false);
    
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


FTSENT* fts_read(FTS* sp) 
{
    register FTSENT *p, *tmp;
    register unsigned short int instr;
    register char* t;

    /* If finished or unrecoverable error, return NULL */
    if(sp->fts_cur == NULL || ISSET(FTS_STOP))
        return NULL;

    /* Set current node pointer */
    p = sp->fts_cur;

    /* Save and zero out user instructions */
    instr = p->fts_instr;
    p->fts_instr = FTS_NOINSTR;

    /* Any type of file may be re-visited; re-stat and return */
    if(instr == FTS_AGAIN)
    {
        p->fts_info = fts_stat(sp, p, false);
        return p;
    }
    Dprintf(("fts_read: p=%s\n",
                p->fts_info == FTS_INIT ? "" : p->fts_path));

    /* Following a symlink -- SLNONE test allows application to see
       SLNONE and recover. If indirecting through a symlink, have
       keep a pointer to current location. If unable to get that
       pointer, follow fails */
    if(instr == FTS_FOLLOW &&
        (p->fts_info == FTS_SL || p->fts_info == FTS_SLNONE))
    {
        p->fts_info = fts_stat(sp, p, true);
        if(p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR))
        {
            if((p->fts_symfd = diropen(sp, ".")) < 0)
            {
                p->fts_errno = errno;
                p->fts_info = FTS_ERR;
            }
            else
                p->fts_flags |= FTS_SYMFOLLOW;
        }
        goto check_for_dir;
    }

    /* Directory in pre-order */
    if(p->fts_info == FTS_D)
    {
        /* If skipped or crossed mount point, do post-order visit */
        if(instr == FTS_SKIP ||
            (ISSET(FTS_XDEV) && p->fts_statp->st_dev != sp->fts_dev))
        {
            if(p->fts_flags & FTS_SYMFOLLOW)
                (void)close(p->fts_symfd);
            if(sp->fts_child)
            {
                fts_lfree(sp->fts_child);
                sp->fts_child = NULL;
            }
            p->fts_info = FTS_DP;
            LEAVE_DIR(sp, p, "1");
            return p;
        }

        /* Rebuild if only read the names and now traversing */
        if(sp->fts_child != NULL && ISSET(FTS_NAMEONLY))
        {
            CLR(FTS_NAMEONLY);
            fts_lfree(sp->fts_child);
            sp->fts_child = NULL;
        }

        /* cd to the subdirectory.

           If have already read and now fail to chdir, whack the list
           to make the names come out right, and set the parent errno
           so the application will eventually get an error condition.
           Set the FTS_DONTCHDIR flag so that when we logically change
           directories back to the parent we don't do chdir.

           If Haven't read do so. If the read fails, fts_build sets
           FTS_STOP or the fts_info field of the node */
        if(sp->fts_child != NULL)
        {
            if(fts_safe_changedir(sp, p, -1, p->fts_accpath))
            {
                p->fts_errno = errno;
                p->fts_flags |= FTS_DONTCHDIR;
                for(p = sp->fts_child; p != NULL; p = p->fts_link)
                    p->fts_accpath = p->fts_parent->fts_accpath;
            }
        }
        else if((sp->fts_child = fts_build(sp, BREAD)) == NULL)
        {
            if(ISSET(FTS_STOP))
                return NULL;

            /* If fts_build's call to fts_safe_changedir failed
               because it was not able to fchdir into a
               subdirectory, tell the caller */
            if(p->fts_errno && p->fts_info != FTS_DNR)
                p->fts_info = FTS_ERR;
            LEAVE_DIR(sp, p, "2");
            return p;
        }
        p = sp->fts_child;
        sp->fts_child = NULL;
        goto name;
    }
    /* Move to the next node on this level */
next:
    tmp = p;
    if((p = p->fts_link) != NULL)
    {
        sp->fts_cur = p;
        free(tmp);

        /* If reached the top, return to the original directory (or
           the root of the tree), and load the file names for the next
           root */
        if(p->fts_level == FTS_ROOTLEVEL)
        {
            if(RESTORE_INITIAL_CWD(sp))
            {
                SET(FTS_STOP);
                return NULL;
            }
            free_dir(sp);
            fts_load(sp, p);
            setup_dir(sp);
            goto check_for_dir;
        }

        /* User may have called fts_set on the node. If skipped,
           ignore. If followed, get a file descriptor so we can
           get back if necessary */
        if(p->fts_instr == FTS_SKIP)
            goto next;
        if(p->fts_instr == FTS_FOLLOW)
        {
            p->fts_info = fts_stat(sp, p, true);
            if(p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR))
            {
                if((p->fts_symfd = diropen(sp, ".")) < 0)
                {
                    p->fts_errno = errno;
                    p->fts_info = FTS_ERR;
                }
                else
                    p->fts_flags |= FTS_SYMFOLLOW;
            }
            p->fts_instr = FTS_NOINSTR;
        }
name:
        t = sp->fts_path + NAPPEND(p->fts_parent);
        *t++ = '/';
        memmove(t, p->fts_name, p->fts_namelen + 1);
check_for_dir:
        sp->fts_cur = p;
        if(p->fts_info == FTS_NSOK)
        {
            if(p->fts_statp->st_size == FTS_STAT_REQUIRED)
            {
                FTSENT* parent = p->fts_parent;
                if(FTS_ROOTLEVEL < p->fts_level
                    /* ->fts_n_dirs_remaining is not valid
                       for command-line-specified names */
                    && parent->fts_n_dirs_remaining == 0
                    && ISSET(FTS_NOSTAT)
                    && ISSET(FTS_PHYSICAL)
                    && link_count_optimize_ok(parent))
                {
                    /* nothing more needed */
                }
                else
                {
                    p->fts_info = fts_stat(sp, p, false);
                    if(S_ISDIR(p->fts_statp->st_mode)
                        && p->fts_level != FTS_ROOTLEVEL
                        && parent->fts_n_dirs_remaining)
                            parent->fts_n_dirs_remaining--;
                }
            }
            else
                fts_assert(p->fts_statp->st_size == FTS_NO_STAT_REQUIRED);
        }
        if(p->fts_info == FTS_D)
        {
            /* Now that P->fts_statp is guaranteed to be valid,
               if this is a command-line directory, record its
               device number, to be used for FTS_XDEV */
            if(p->fts_level == FTS_ROOTLEVEL)
                sp->fts_dev = p->fts_statp->st_dev;
            Dprintf(("  entering: %s\n", p->fts_path));
            if(!enter_dir(sp, p))
            {
                __set_errno(ENOMEM);
                return NULL;
            }
        }
        return p;
    }

    /* Move up to the parent node */
    p = tmp->fts_parent;
    sp->fts_cur = p;
    free(tmp);

    if(p->fts_level == FTS_ROOTPARENTLEVEL)
    {
        /* Done; free everything up and set errno to 0 so the user
           can distinguish between error and EOF */
        free(p);
        __set_errno(0);
        return (sp->fts_cur = NULL);
    }

    fts_assert(p->fts_info != FTS_NSOK);

    /* NUL terminate the file name */
    sp->fts_path[p->fts_pathlen] = '\0';

    /* Return to the parent directory. If at a root node, restore
       the initial working directory. If we came through a symlink,
       go back through the file descriptor. Otherwise, move up
       one level, via ".." */
    if(p->fts_level == FTS_ROOTLEVEL)
    {
        if(RESTORE_INITIAL_CWD(sp))
        {
            p->fts_errno = errno;
            SET(FTS_STOP);
        }
    }
    else if(p->fts_flags & FTS_SYMFOLLOW)
    {
        if(FCHDIR(sp, p->fts_symfd))
        {
            int saved_errno = errno;
            (void)close(p->fts_symfd);
            __set_errno(saved_errno);
            p->fts_errno = errno;
            SET(FTS_STOP);
        }
        (void)close(p->fts_symfd);
    }
    else if(!(p->fts_flags & FTS_DONTCHDIR) &&
                fts_safe_changedir(sp, p->fts_parent, -1, ".."))
    {
        p->fts_errno = errno;
        SET(FTS_STOP);
    }
    p->fts_info = p->fts_errno ? FTS_ERR : FTS_DP;
    if(p->fts_errno == 0)
        LEAVE_DIR(sp, p, "3");
    return ISSET(FTS_STOP) ? NULL : p;
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
