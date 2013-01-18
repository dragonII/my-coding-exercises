/* Determine whether two stat buffers refer to the same file */

#ifndef SAME_INODE_H
#define SAME_INODE_H

#define SAME_INODE(Stat_buf_1, Stat_buf_2) \
        ((Stat_buf_1).st_ino == (Stat_buf_2).st_ino \
         && (Stat_buf_1).st_dev == (Stat_buf_2).st_dev)

#endif
