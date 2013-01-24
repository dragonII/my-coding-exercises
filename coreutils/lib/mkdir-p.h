/* mkdir-p.h -- Ensure that a directory and its parents exist */

#ifndef MKDIR_P_H
#define MKDIR_P_H

#include <stdbool.h>
#include <sys/types.h>

struct savewd;
bool make_dir_parents(char* dir,
                      struct savewd* wd,
                      int (*make_ancestor)(char*, char*, void*),
                      void* options,
                      mode_t mode,
                      void (*announce)(char*, void*),
                      mode_t mode_bits,
                      uid_t owner,
                      gid_t group,
                      bool preserve_existing);

#endif
