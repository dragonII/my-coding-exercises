/* Traverse a file hierarchy */

#ifndef _FTS_H
#define _FTS_H

typedef struct 
{
    struct _ftsent* fts_cur;        /* current node */
    struct _ftsent* fts_child;      /* linked list of children */
    struct _ftsent** fts_array;     /* sort array */
    dev_t  fts_dev;                 /* starting device# */
    char* fts_path;                 /* file name for this desent */
    int fts_rfd;                    /* fd for root */
    int fts_cwd_fd;                 /* the file descriptor on which the
                                       virtual cwd is open, or AT_FDCWD */
    size_t fts_pathlen;             /* sizeof(path) */
    size_t fts_nitems;              /* elements in the sort array */
    int (*fts_compar) (struct _ftsent **, struct _ftsent **);
                                    /* compare fn */

#define FTS_COMFOLLOW   0x0001      /* follow command line symlinks */
#define FTS_LOGICAL     0x0002      /* logical walk */
#define FTS_NOCHDIR     0x0004      /* don't change directories */
#define FTS_NOSTAT      0x0008      /* don't get stat info */
#define FTS_PHYSICAL    0x0010      /* physical walk */
#define FTS_SEEDOT      0x0020      /* return dot and dot-dot */
#define FTS_XDEV        0x0040      /* don't cross devices */
#define FTS_WHITEOUT    0x0080      /* return whiteout information */

/* Historically, for each directory that fts initially encounters, it would
   open it, read all entries, and stat each entry, storing the results, and
   then it would process the first entry. But that behavior is bad for
   locality of reference, and also causes trouble with inode-simulating
   file system like FAT, CIFS, FUSE-based ones, etc., when entries from
   their name/inode chche are flushed too early.
   Use the flag to make fts_open and fts_read defer the stat/lstat/fststat
   of each entry until it is actually processed. However, note that if you
   use this option and also spcify a comparison function, that function may
   not examine any data via fts_statp. However, when fts_statp->st_mode is
   nonzero, the S_IMFT type bits are valid, with mapped dirend.d_type data.
   Of course, that happens only on file systems that provide useful
   dirent.d_type data. */
#define FTS_DEFER_STAT  0x0400

} FTS;



#endif
