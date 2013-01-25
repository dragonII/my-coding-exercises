/* Make a files' ancestor directories */

#include "mkancesdirs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fnctl.h>

#include <stddef.h>
#include <errno.h>
#include <unistd.h>

#include "dirname.h"
#include "savewd.h"

/* Ensure that the ancestor directories of FILE exist, using an
   algorithm that should work even if two processes execute this
   function in parallel. Modify FILE as necessary to access the
   ancestor directories, but restore FILE to an equivalent value
   if successful.

   WD points to the working directory, using the conventions of
   savewd.

   Create any ancestor directories that don't already exist, by
   invoking MAKE_DIR (FILE, COMPONENT, MAKE_DIR_ARG). This function
   should return 0 if successful and the resulting directory is
   readable, 1 if successfu but the resulting directory might not
   be readable, -1 (setting errno) otherwise. If COMPONENT is relative,
   it is relative to the temporary working directory, which may differ
   from *WD.

   Ordinarily MAKE_DIR is executed with the working directory changed
   to reflect the already-made prefix, and mkancesdirs returns with
   the working directory changed a prefix of FILE. However, if the
   initial working directory cannot be saved in a file descriptor,
   MAKE_DIR is invoked in a subprocess and this function returns in
   both the parent and child process, so the caller should not assume
   any changed state survives other than the EXITMAX component of WD,
   and the caller should take care that the parent does not attempt to
   do the work that the child is doing.

   If successful and if this process can go ahead and create FILE,
   return the length of the prefix of FILE that has already been made.
   If successful so far but a child process is doing the actual work,
   return -2. If unsuccessful, return -1 and set errno */
