/* Detect cycles in file tree walks */

#ifndef FTS_CYCLE_HEADER
#define FTS_CYCLE_HEADER

#include "fts_.h"


void free_dir(FTS* sp);
bool setup_dir(FTS* fts);
void leave_dir(FTS* fts, FTSENT* ent);
bool enter_dir(FTS* fts, FTSENT* ent);
bool cycle_check(struct cycle_check_state*, struct stat*);


#define CYCLE_CHECK_REFLECT_CHDIR_UP(State, SB_dir, SB_subdir)  \
    do                                                          \
    {                                                           \
        /* You must call cycle_check at least once before using this macro*/ \
        if((State)->chdir_counter == 0)                         \
            abort();                                            \
        if(SAME_INODE((State)->dev_ino, SB_subdir))             \
        {                                                       \
            (State)->dev_ino.st_dev = (SB_subdir).st_dev;       \
            (State)->dev_ino.st_ino = (SB_subdir).st_ino;       \
        }                                                       \
    } while(0)

#endif
