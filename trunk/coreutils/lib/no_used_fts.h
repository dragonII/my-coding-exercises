#include <stdbool.h>
#include "fts.h"

FTS*
xfts_open(char**, int options,
          int (*)(FTSENT**, FTSENT**));
