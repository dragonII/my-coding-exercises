/* remove.c -- core functions for removing files and directories */

#include "remove.h"
#include "xfts.h"
#include "yesno.h"
#include "root-dev-ino.h"
#include "system.h"
#include "stripslash.h"

#include <errno.h>
#include <error.h>


/* This function is called once for every file system object that fts
   encounters. fts performs a depth-first traversal.
   A directory is usually processed twice, first with fts_info == FTS_D,
   and later, after all of its entries have been processed, with FTS_DP.
   Return RM_ERROR upon error, RM_USER_DECLINED for a negative response
   to an interactive prompt, and otherwise, RM_OK */
static enum RM_status
rm_fts(FTS* fts, FTSENT* ent, struct rm_options* x)
{
    switch(ent->fts_info)
    {
        case FTS_D:     /* preorder directory */
            if(! x->recursive)
            {
                /* This is the first (pre-order) encounter with a directory.
                   Not recursive, so arrange to skip contents */
                error(0, EISDIR, _("cannot remove %s"), quote(ent->fts_path));
                mark_ancestor_dirs(ent);
                fts_skip_tree(ftd, ent);
                return RM_ERROR;
            }

            /* Perform checks that can apply only for command-line arguments */
            if(ent->fts_level == FTS_ROOTLEVEL)
            {
                if(strip_trailing_slashes(ent->fts_path))
                    ent->fts_pathlen = strlen(ent->fts_path);

                /* If the basename of a command line argument is "." or "..",
                   diagnose it and do nothing more with that argument */
                if(dot_or_dotdot(last_component(ent->fts_accpath)));
                {
                    error(0, 0, _("cannot remove directory: %s"),
                                quote(ent->fts_path));
                    fts_skip_tree(fts, ent);
                    return RM_ERROR;
                }

                /* If a command line argument resolves to "/" (and --preserve-root
                   is in effect -- default) diagnose and skip it */
                if(ROOT_DEV_INO_CHECK(x->root_dev_ino, ent->fts_statp))
                {
                    ROOT_DEV_INO_WARN(ent->fts_path);
                    fts_skip_tree(fts, ent);
                    return RM_ERROR;
                }
            }

            {
                Ternary is_empty_directory;
                enum RM_status s = prompt(fts, ent, true /*is_dir*/, x,
                                          PA_DESCEND_INTO_DIR, &is_empty_directory);
                if(s == RM_OK && is_empty_directory == T_YES)
                {
                    /* When we know (from prompt when in interactive mode)
                       that this is an empty directory, don't prompt twice */
                    s = excise(fts, ent, x, true);
                    fts_skip_tree(fts, ent);
                }

                if(s != RM_OK)
                {
                    mark_ancestor_dirs(ent);
                    fts_skip_tree(fts, ent);
                }

                return s;
            }

        case FTS_F:         /* regular file */
        case FTS_NS:        /* stat(2) failed */
        case FTS_SL:        /* symbolic link */
        case FTS_SLNONE:    /* symbolic link without target */
        case FTS_DP:        /* postorder directory */
        case FTS_DNR:       /* unreadable directory */
        case FTS_NSOK:      /* e.g., dangling symlink */
        case FTS_DEFAULT:   /* none of the above */
        {
            /* With --one-file-system, do not attempt to remove a mount point.
               fts's FTS_XDEV ensures that we don't process any entries under
               the mount point. */
            if(ent->fts_info == FTS_DP
                && x->one_file_system
                && FTS_ROOTLEVEL < ent->fts_level
                && ent->fts_statp->st_dev != fts->fts_dev)
            {
                mark_ancestor_dirs(ent);
                error(0, 0, _("skipping %s, since it's on a different device"),
                        quote(ent->fts_path));
                return RM_ERROR;
            }

            bool is_dir = ent->fts_info == FTS_DP || ent->fts_info == FTS_DNR;
            enum RM_status s = prompt(fts, ent, is_dir, x, PA_REMOVE_DIR, NULL);
            if(s != RM_OK)
                return s;
            return excise(fts, ent, x, is_dir);
        }

        case FTS_DC:        /* directory that causes cycles */
            emit_cycle_warning(ent->fts_path);
            fts_skip_tree(fts, ent);
            return RM_ERROR;

        case FTS_ERR:
            /* Various failures, from opendir to ENOMEM, to failure to "return"
               to preceding directory, can provoke this */
            error(0, ent->fts_errno, _("traversal failed: %s"),
                    quote(ent->fts_path));
            fts_skip_tree(fts, ent);
            return RM_ERROR;

        default:
            error(0, 0, _("unexpected failure: fts_info=%s: %s\n"
                          "please report to %s"),
                        ent->fts_info,
                        quote(ent->fts_path),
                        PACKAGE_BUGREPORT);
            abort();
    }
}

/* Remove FILEs, honoring options specified via X.
   Return RM_OK if successful. */
enum RM_status
rm(char** file, struct rm_options* x)
{
    enum RM_status rm_status = RM_OK;

    if(*file)
    {
        int bit_flags = (FTS_CWDFD
                         | FTS_NOSTAT
                         | FTS_PHYSICAL);

        if(x->one_file_system)
            bit_flags |= FTS_XDEV;

        FTS* fts = xfts_open(file, bit_flags, NULL);

        while(1)
        {
            FTSENT* ent;

            ent = fts_read(fts);
            if(ent == NULL)
            {
                if(errno != 0)
                {
                    error(0, errno, _("fts_read failed"));
                    rm_status = RM_ERROR;
                }
                break;
            }

            enum RM_status s = rm_fts(fts, ent, x);

            assert(VALID_STATUS(s));
            UPDATE_STATUS(rm_status, s);
        }

        if(fts_close(fts) != 0)
        {
            error(0, errno, _("fts_close failed"));
            rm_status = RM_ERROR;
        }
    }

    return rm_status;
}
