/* userspec.c -- Parse a user and group string */

#include "userspec.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <sys/param.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "intprops.h"
#include "xalloc.h"
#include "xstrtol.h"
#include "inttostr.h"

#include "gettext.h"
#define _(msgid) gettext(msgid)
#define N_(msgid) msgid

#ifndef UID_T_MAX
# define UID_T_MAX TYPE_MAXIMUM (uid_t)
#endif

#ifndef GID_T_MAX
# define GID_T_MAX TYPE_MAXIMUM (gid_t)
#endif

/* MAXUID may come from limits.h or sys/param.h */
#ifndef MAXUID
# define MAXUID UID_T_MAX
#endif
#ifndef MAXGID
# define MAXGID GID_T_MAX
#endif

static char*
parse_with_separator(char* spec, char* separator,
                     uid_t* uid, gid_t* gid,
                     char** username, char** groupname)
{
    static char* E_invalid_user = N_("invalid user");
    static char* E_invalid_group = N_("invalid group");
    static char* E_bad_spec = N_("invalid spec");

    char* error_msg;
    struct passwd* pwd;
    struct group* grp;
    char* u;
    char* g;
    char* gname = NULL;
    uid_t unum = *uid;
    gid_t gnum = *gid;

    error_msg = NULL;
    *username = *groupname = NULL;

    /* Set U and G to nonzero length strings corresponding to user and
       group specifiers or to NULL. If U is not NULL, it is a newly
       allocated string. */

    u = NULL;
    if(separator == NULL)
    {
        if(*spec)
            u = xstrdup(spec);
    }
    else
    {
        size_t ulen = separator - spec;
        if(ulen != 0)
        {
            u = xmemdup(spec, ulen + 1);
            u[ulen] = '\0';
        }
    }

    g = (separator == NULL || *(separator + 1) == '\0'
            ? NULL
            : separator + 1);

    if(u != NULL)
    {
        /* If it starts with "+", skip the look up */
        pwd = (*u == '+' ? NULL : getpwnam(u));
        if(pwd == NULL)
        {
            bool use_login_group = (separator != NULL && g == NULL);
            if(use_login_group)
            {
                /* If there is no group,
                   then there may not be a trailing ":", either */
                error_msg = E_bad_spec;
            }
            else
            {
                unsigned long int tmp;
                if(xstrtoul(u, NULL, 10, &tmp, "") == LONGINT_OK
                        && tmp <= MAXUID && (uid_t)tmp != (uid_t) -1)
                    unum = tmp;
                else
                    error_msg = E_invalid_user;
            }
        }
        else
        {
            unum = pwd->pw_uid;
            if(g == NULL && separator != NULL)
            {
                /* A separator was given, but a group was not specified,
                   so get the login group. */
                char buf[INT_BUFSIZE_BOUND (uintmax_t)];
                gnum = pwd->pw_gid;
                grp = getgrgid(gnum);
                gname = xstrdup(grp ? grp->gr_name : umaxtostr(gnum, buf));
                endgrent();
            }
        }
        endpwent();
    }

    if(g != NULL && error_msg == NULL)
    {
        /* Explicit group */
        /* If it starts with "+", skip the look-up */
        grp = (*g == '+' ? NULL : getgrnam(g));
        if(grp == NULL)
        {
            unsigned long int tmp;
            if(xstrtoul(g, NULL, 10, &tmp, "") == LONGINT_OK
                    && tmp <= MAXGID && (gid_t)tmp != (gid_t) -1)
                gnum = tmp;
            else
                error_msg = E_invalid_group;
        }
        else
            gnum = grp->gr_gid;

        endgrent();     /* Save a file descriptor */
        gname = xstrdup(g);
    }

    if(error_msg == NULL)
    {
        *uid = unum;
        *gid = gnum;
        *username = u;
        *groupname = gname;
        u = NULL;
    }
    else
        free(gname);

    free(u);

    return (char*)_(error_msg);
}


/* Extract from SPEC, which has the form "[user][:.][group]",
   a USERNAME, UID U, GROUPNAME, and GID G.
   Either user or group, or both, must be present.
   If the group is omitted but the separator is given,
   use the given user's login group.
   If SPEC contains a `:', then use that as the separator, ignoring
   any `.'s. If there is no `:', but there is a `.', then first look
   up the entire SPEC as a login name. If that look-up fails, then
   try again interpreting the `.' as a separator.

   USERNAME and GROUPNAME will be in newly malloc'd memory.
   Either one might be NULL instead, indicating that it was not
   given and the corresponding numeric ID was left unchanged.

   Return NULL if successful, a static error message string if not. */
char* parse_user_spec(char* spec, uid_t* uid, gid_t* gid,
                        char** username, char** groupname)
{
    char* colon = strchr(spec, ':');
    char* error_msg = 
            parse_with_separator(spec, colon, uid, gid, username, groupname);

    if(!colon && error_msg)
    {
        /* If there's no colon but there is a dot, and if looking up the
           whole spec failed (i.e., the spec is not a owner name that
           includes a dot), then try again, but interpret the dot as a
           separator. This is a compatible extension to POSIX, since
           the POSIX-required behavior is always tried first */
        char* dot = strchr(spec, '.');
        if(dot
             && ! parse_with_separator(spec, dot, uid, gid, username, groupname))
            error_msg = NULL;
    }

    return error_msg;
}
