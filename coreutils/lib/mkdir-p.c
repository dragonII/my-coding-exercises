/* mkdir-p.c -- Ensure that a directory and its parents exist. */

#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "savewd.h"
#include "dirname.h"

/* Ensure that the directory DIR exist.

   WD is the working directory, as in savewd.c.

   If MAKE_ANCESTOR is not null, create any ancestor directories that
   don't already exist, by invoking MAKE_ANCESTOR (DIR, ANCESTOR, OPTIONS).
   This function should return zero if successful, -1 (setting errno)
   otherwise. In this case, DIR may be modified by storing '\0' bytes
   into it, to access the ancestor directories, and this modification
   is retained on return if the ancestor directories could not be
   created.

   Create DIR as a new directory with using mkdir with permissions
   MODE. It is also OK if MAKE_ANCESTOR is not null and a
   directory DIR already exists.

   Call ANNOUNCE (DIR, OPTIONS) just after successfully making DIR,
   even if some of the following actions fail.

   Set DIR's owner to OWNER and group to GROUP, but leave the owner
   alone if OWNER is (uid_t) -1, and similarly for GROUP.

   Set DIR's mode bits to MODE, except preserve any of the bits that
   correspond to zero bits in MODE_BITS. In other words, MODE_BITS is
   a mask that specifies which of DIR's mode bits should be set or 
   cleared. MODE should be a subset of MODE_BITS, which in turn
   should be a subset of CHMOD_MODE_BITS. Changing the mode in this
   way is necessary if DIR already existed or if MODE and MODE_BITS
   specify non-permissions bits like S_ISUID.

   However, if PRESERVE_EXISTING is true and DIR already exists,
   do not attempt to set DIR's ownership and file mode bits.

   This implementation assumes the current umask to zero.

   Return true if DIR exists and a directory with the proper ownership
   and file mode bits when done, or if a child process has been
   dispatched to do the read work (though the child process may not
   have finished yet -- it is the caller's responsibility to handle
   this). Report a diagnostic and return false on failure, storing
   '\0' into *DIR if an ancestor directory had problems. */
bool make_dir_parents(char* dir,
                      struct savewd* wd,
                      int (*make_ancestor)(char*, char*, void*),
                      void* options,
                      mode_t mode,
                      void (*announce)(char*, void*),
                      mode_t mode_bits,
                      uid_t owner,
                      gid_t group,
                      bool preserve_existing)
{
    int mkdir_errno = (IS_ABSOLUTE_FILE_NAME(dir) ? 0 : savewd_errno(wd));

    if(mkdir_errno == 0)
    {
        ptrdiff_t prefix_len = 0;
        int savewd_chdir_options = (HAVE_FCHMOD ? SAVEWD_CHDIR_SKIP_READABLE : 0);

        if(make_ancestor)
        {
            prefix_len = mkancesdirs(dir, wd, make_ancestor, options);
            if(prefix_len < 0)
            {
                if(prefix_len < -1)
                    return true;
                mkdir_errno = errno;
            }
        }

        if(prefix_len >= 0)
        {
            /* If the ownership might change, or if the directory will be
               writable to other users and its special mode bits may
               change after the directory is created, create it with
               more restrictive permissions at first, so unauthorized
               users cannot nip in before the directory is ready */
            bool keep_owner = owner == (uid_t) -1 && group == (gid_t) -1;
            bool keep_special_mode_bits = 
                    ((mode_bits & (S_ISUID | S_ISGID))
