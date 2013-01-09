/* mgetgroups.c -- return a list of the groups a user or current process is in */

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <grp.h>

#include "xalloc.h"

/* Store the result in malloc'd storage.
   Set *GROUPS to the malloc'd list of all group IDs of which USERNAME
   is a member. If GID is not -1, store it first. GID should be the
   group ID (pwd_gid) obtained from getpwuid, in case USERNAME is not
   listed in the groups database (e.g., /etc/groups). If USERNAME is
   NULL, store the supplementary groups of the current process, and GID
   should be -1 or the effective group ID (getegid). Upon failure,
   don't modify *GROUPS, set errno, and return -1. Otherwise, return
   the number of groups. The resulting list may contain duplicates,
   but adjacent members will be distinct. */
int mgetgroups(char* username, gid_t gid, gid_t** groups)
{
    int max_n_groups;
    int ng;
    gid_t* g;

    if(username)
    {
        enum { N_GROUPS_INIT = 10 };
        max_n_groups = N_GROUPS_INIT;

        g = realloc_groupbuf(NULL, max_n_groups);
        if(g == NULL)
            return -1;

        while(1)
        {
            gid_t *h;
            int last_n_groups = max_n_groups;

            /* getgrouplist updates max_n_groups to num required */
            ng = getgrouplist(username, gid, g, &max_n_groups);

            /* Some systems(like Darwin) have a bug where they never 
               increase max_n_groups */
            if(ng < 0 && last_n_groups == max_n_groups)
                max_n_groups *= 2;

            if((h = realloc_groupbuf(g, max_n_groups)) == NULL)
            {
                int saved_errno = errno;
                free(g);
                errno = saved_errno;
                return -1;
            }
            g = h;

            if(ng >= 0)
            {
                *groups = g;
                /* On success some systems just return 0 from getgrouplist,
                   so return max_n_groups rather than ng. */
                return max_n_groups;
            }
        }
    }
    /* else no username, so fall through and use getgroups. */
    max_n_groups = (username  
                        ? getugroups(0, NULL, username, gid)
                        : getgroups(0, NULL));

    /* If we failed to count groups because there is no supplemental
       group support, then return an array containing just GID.
       Otherwise, we fail for the same reason. */
    if(max_n_groups < 0)
    {
        if(errno == ENOSYS && (g = realloc_groupbuf(NULL, 1)))
        {
            *group = g;
            *g = gid;
            return gid != (gid_t) -1;
        }
        return -1;
    }

    if(!username && gid != (gid_t) -1)
        max_n_groups++;
    g = realloc_groupbuf(NULL, max_n_groups);
    if(g == NULL)
        return -1;

    ng = (username
            ? getugroups(max_n_groups, g, username, gid)
            : getgroups(max_n_groups - (gid != (gid_t) -1),
                            g + (gid != (gid_t) -1)));
    if(ng < 0)
    {
        /* Failure is unexpected, but handle it anyway. */
        int saved_errno = errno;
        free(g);
        errno = saved_errno;
        return -1;
    }

    if(!username && gid != (gid_t) -1)
    {
        *g = gid;
        ng++;
    }
    *groups = g;

    /* Reduce the number of duplicates. On some systems, getgroups
       returns the effective gid twice: once as the first element, and
       once in its position within the supplementary groups. On other
       systems, getgroups does not return the effective gid at all,
       which is why we provide a GID argument. Meanwhile, the GID
       argument, if provided, is typically any member of the
       supplementary groups, and not necessarily the effective gid. So,
       the most likely duplicates are the first element with an 
       arbitrary other element, or pair-wire duplication between the
       first and second elements returned by getgroups. It is possible
       that this O(n) pass will not remove all duplicates, but it is not
       worth the effort to slow down to an O(n log n) algorithm that
       sorts the array in place, nor the extra memory needed for
       duplicate removal via an O(n) hast-table. Hence, this function
       is only documented as guaranteeing no pair-wise duplicates,
       rahter than returning the minimal set. */

    if(ng > 1)
    {
        gid_t first = *g;
        gid_t* next;
        gid_t* groups_end = g + ng;

        for(next = g + 1; next < groups_end; next++)
        {
            if(*next == first || *next == *g)
                ng--;
            else
                *++g = *next;
        }
    }
    return ng;
}



int xgetgroups(char* username, gid_t gid, gid_t** groups)
{
    int result = mgetgroups(username, gid, groups);
    if(result == -1 && errno == ENOMEM)
        xalloc_die();

    return result;
}
