/* chown-core.c -- core functions for changing ownership */

#include "chown-core.h"
#include "xalloc.h"
#include "intprops.h"
#include "inttostr.h"
#include "fts.h"


#include <grp.h>

void chopt_init(struct Chown_option* chopt)
{
    chopt->verbosity = V_off;
    chopt->root_dev_ino = NULL;
    chopt->affect_symlink_referent = true;
    chopt->recurse = false;
    chopt->force_silent = false;
    chopt->user_name = NULL;
    chopt->group_name = NULL;
}

/* Convert the numeric group-id, GID, to a string stored in xmalloc'd memory,
   and return it. If there's no corresponding group name, use the decimal
   representation of the ID. */
char* gid_to_name(gid_t gid)
{
    char buf[INT_BUFSIZE_BOUND(intmax_t)];
    struct group *grp = getgrgid(gid);
    return xstrdup(grp ? grp->gr_name
                    : TYPE_SIGNED(gid_t) ? imaxtostr(gid, buf)
                    : umaxtostr(gid, buf));
}


/* Change the owner and/or group of the specified FILES.
   BIT_FLAGS specifies how to treat each symlink-to-directory
   that is encountered during a recursive traversal.
   CHOPT specifies additional options.
   If UID is not -1, then change the owner id of each file to UID.
   If GID is not -1, then change the group id of each file to GID.
   If REQUIRED_UID and/or REQUIRED_GID is not -1, then change only
   files with user ID and group ID that match the non-(-1) value(s).
   Return true if successful. */
bool chown_files(char** files, int bit_flags,
                 uid_t uid, gid_t gid,
                 uid_t required_uid, gid_t required_gid,
                 struct Chown_option* chopt)
{
    bool ok = true;

    /* Use lstat and stat only if they're needed */
    int stat_flags = ((required_uid != (uid_t) -1 || required_gid != (gid_t) -1
                        || chopt->affect_symlink_referent
                        || chopt->verbosity != V_off)
                      ? 0
                      : FTS_NOSTAT);
    FTS* fts = xfts_open(files, bit_flags | stat_flags, NULL);

    while(1)
    {
        FTSENT* ent;

        ent = fts_read(fts);
        if(ent == NULL)
        {
            if(errno != 0)
            {
                if(!chopt->force_silent)
                    error(0, errno, _("fts_read failed"));
                ok = false;
            }
            break;
        }
        ok &= change_file_owner(fts, ent, uid, gid,
                                required_uid, required_gid, chopt);
    }

    if(fts_close(fts) != 0)
    {
        error(0, errno, _("fts_close failed"));
        ok = false;
    }

    return ok;
}

void chopt_free(struct Chown_option* chopt ATTRIBUTE_UNUSED)
{
    /* Deliberately do not free chopt->user_name or ->group_name.
       They're not always allocated. */
}
