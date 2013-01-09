/* group-list.c -- Print a list of group IDs or names. */

#include <stdbool.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <grp.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>

#include "group-list.h"
#include "mgetgroups.h"
#include "quote.h"

#include "gettext.h"
#define _(msgid) gettext(msgid)

/* Print the name or value of group ID GID. */
bool print_group(gid_t gid, bool use_names)
{
    struct group *grp = NULL;
    bool ok = true;

    if(use_names)
    {
        grp = getgrgid(gid);
        if(grp == NULL)
        {
            error(0, 0, _("cannot find name for group ID %lu"),
                    (unsigned long int)gid);
            ok = false;
        }
    }

    if(grp == NULL)
        printf("%lu", (unsigned long int)gid);
    else
        printf("%s", grp->gr_name);

    return ok;
}

/* Print all of the distinct groups the users is in. */
bool print_group_list(const char* username,
                      uid_t ruid, gid_t rgid, gid_t egid,
                      bool use_names)
{
    bool ok = true;
    struct passwd* pwd;

    pwd = getpwuid(ruid);
    if(pwd == NULL)
        ok = false;

    if(!print_group(rgid, use_names))
        ok = false;

    if(egid != rgid)
    {
        putchar(' ');
        if(!print_group(egid, use_names))
            ok = false;
    }
    
    {
        gid_t* groups;
        int i;

        int n_groups = xgetgroups(username, (pwd ? pwd->pw_gid : (gid_t) -1),
                                    &groups);
        if(n_groups < 0)
        {
            if(username)
            {
                error(0, errno, _("failed to get groups for user %s"),
                        quote(username));
            }
            else
            {
                error(0, errno, _("failed to get groups for the current process"));
            }
            return false;
        }

        for(i = 0; i < n_groups; i++)
            if(groups[i] != rgid && groups[i] != egid)
            {
                putchar(' ');
                if(!print_group(groups[i], use_names))
                    ok = false;
            }
        free(groups);
    }
    return ok;
}

