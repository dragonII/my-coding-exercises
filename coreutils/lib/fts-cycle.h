/* Detect cycles in file tree walks */

#ifndef FTS_CYCLE_HEADER
#define FTS_CYCLE_HEADER

#include "fts_.h"


void free_dir(FTS* sp);
bool setup_dir(FTS* fts);
void leave_dir(FTS* fts, FTSENT* ent);


#endif
