/* getugroups.c -- return a list of the groups a user is in */

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>

/* Like getgroups, but for user USERNAME instead of for the current
   process. Store at most MAXCOUNT group IDs in the GROUPLIST array.
   If GID is not -1, store it first (if possible). GID should be the
   group ID (pw_gid) obtained from getpwuid, in case USERNAME is not
   listed in /etc/groups. Upon failure, set errno and return -1.
   Otherwise, return the number of IDs we've written into GROUPLIST. */
int getugroups(int maxcount, gid_t* grouplist, const char* username, gid_t gid)
{
    int count = 0;
    if(gid != (gid_t) -1)
    {
        if(maxcount != 0)
            grouplist[count] = gid;
        ++count;
    }

    setgrent();
    while(1)
    {
        char** cp;
        struct group *grp;

        errno = 0;
        grp = getgrent();
        if(grp == NULL)
            break;

        for(cp = grp->gr_mem; *cp; ++cp)
        {
            int n;
            //if(!STREQ(username, *cp))
            if(strcmp(username, *cp) != 0)
                continue;

            /* See if this group number is already on the list */
            for(n = 0; n < count; ++n)
                if(grouplist && grouplist[n] == grp->gr_gid)
                    break;

            /* If it's a new group number, then try to add it to the list */
            if(n == count)
            {
                if(maxcount != 0)
                {
                    if(count >= maxcount)
                        goto done;
                    grouplist[count] = grp->gr_gid;
                }
                if(count == INT_MAX)
                {
                    errno = EOVERFLOW;
                    goto done;
                }
                count++;
            }
        }
    }
    if(errno != 0)
        count = -1;

    done:
    {
        int saved_errno = errno;
        endgrent();
        errno = saved_errno;
    }

    return count;
}
