#include <stdbool.h>
#include "fts_.h"

FTS*
xfts_open(char**, int options,
          int (*)(FTSENT**, FTSENT**));
