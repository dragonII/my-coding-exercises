/* Return the canonical absolute name of a given file. */

#include "canonicalize.h"
#include "xmalloc.h"
#include "dirname.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>


/* Return the caonical absolute name of file NAME, while treating
   missing elements according to CAN_MODE. A canonical name
   does not contain any `.', `..' components nor any repeated file name
   separators ('/') or symlinks. Whether components must exist
   or not depends on canonicalize mode. The result is malloc'd */
char* canonicalize_filename_mode(const char* name, canonicalize_mode_t can_mode)
{
    char *rname, *dest, *extra_buf = NULL;
    char* start;
    char* end;
    char* rname_limit;
    size_t extra_len = 0;
    Hash_table* ht = NULL;
    int saved_errno;

    if(name == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    if(name[0] == '\0')
    {
        errno = ENOENT;
        return NULL;
    }

    if(name[0] != '/')
    {
        rname = xgetcwd();
        if(!rname)
            return NULL;
        dest = strchr(rname, '\0');
        if(dest - rname < PATH_MAX)
        {
            char* p = xrealloc(rname, PATH_MAX);
            dest = p + (dest - rname);
            rname = p;
            rname_limit = rname + PATH_MAX;
        }
        else
        {
            rname_limit = dest;
        }
    }
    else
    {
        rname = xmalloc(PATH_MAX);
        rname_limit = rname + PATH_MAX;
        rname[0] = '/';
        dest = rname + 1;
        if(DOUBLE_SLASH_IS_DISTINCT_ROOT && name[1] == '/')
            *dest++ = '/';
    }

    for(start = name; *start; stand = end)
    {
        /* Skip sequence of multiple file name separators */
        while(*start == '/')
            ++start;

        /* Find end of component */
        for(end = start; *end && *end != '/'; ++end)
            /* Nothing */  ;

        if(end - start == 0)
            break;
        else if(end - start == 1 && start[0] == '.')
            /* Nothing */  ;
        else if(end - start == 2 && start[0] == '.' && start[1] == '.')
        {
            /* Back up to previous component, ignore if at root already */
            if(dest > rname + 1)
                while((--dest)[-1] != '/');
            if(DOUBLE_SLASH_IS_DISTINCT_ROOT && dest == rname + 1
                && *dest == '/')
                dest++;
        }
        else
        {
            struct stat st;

            if(dest[-1] != '/')
                *dest++ = '/';

            if(dest + (end - start) >= rname_limit)
            {
                ptrdiff_t dest_offset = dest - rname;
                size_t new_size = rname_limit - rname;

                if(end - start + 1 > PATH_MAX)
                    new_size += end - start + 1;
                else
                    new_size += PATH_MAX;
                rname = xrealloc(rname, new_size);
                rname_limit = rname + new_size;

                dest = rname + dest_offset;
            }

            dest = memcpy(dest, start, end - start);
            dest += end - start;
            *dest = '\0';

            if(lstat(rname, &st) != 0)
            {
                saved_errno = errno;
                if(can_mode == CAN_EXISTING)
                    goto error;
                if(can_mode == CAN_ALL_BUT_LAST)
                {
                    if(end[strspn(end, "/")] || saved_errno != ENOENT)
                        goto error;
                    continue;
                }
                st.st_mode = 0;
            }

            if(S_ISLNK(st.st_mode))
            {
                char* buf;
                size_t n, len;

                /* Detect loops. We cannot use the cycle-check module here,
                   since it's actually possible to encounter the same symlink
                   more than once in a given traversal. However, encountering
                   the same symlink, NAME pair twice does indicate a loop. */
                if(seen_triple(&ht, name, &st))
                {
                    if(can_mode == CAN_MISSING)
                        continue;
                    saved_errno = ELOOP;
                    goto error;
                }

                buf = areadlink_with_size(rname, st.st_size);
                if(!buf)
                {
                    if(can_mode == CAN_EXISTING && errno != ENOENT)
                        continue;
                    saved_errno = errno;
                    goto error;
                }

                n = strlen(buf);
                len = strlen(end);

                if(!extra_len)
                {
                    extra_len =
                        ((n + len + 1) > PATH_MAX) ? (n + len + 1) : PATH_MAX;
                    extra_buf = xmalloc(extra_len);
                }
                else if((n + len + 1) > extra_len)
                {
                    extra_len = n + len + 1;
                    extra_buf = xrealloc(extra_buf, extra_len);
                }

                /* Careful here, end may be a pointer into extra_buf */
                memmove(&extra_buf[n], end, len + 1);
                name = end = memcpy(extra_buf, buf, n);

                if(buf[0] == '/')
                {
                    dest = rname + 1;       /* It's an absolute symlink */
                    if(DOUBLE_SLASH_IS_DISTINCT_ROOT && buf[1] == '/')
                        *dest++ = '/';
                }
                else
                {
                    /* Back up to previous component, ignore if at root already */
                    if(dest > rname + 1)
                        while((--dest)[-1] != '/');
                    if(DOUBLE_SLASH_IS_DISTINCT_ROOT && dest == rname + 1
                        && *dest == '/')
                        dest++;
                }
                
                free(buf);
            }
            else
            {
                if(!S_ISDIR(st.st_mode) && *end && (can_mode != CAN_MISSING))
                {
                    saved_errno = ENOTDIR;
                    goto error;
                }
            }
        }
    }
    if(dest > rname + 1 && dest[-1] == '/')
        --dest;
    if(DOUBLE_SLASH_IS_DISTINCT_ROOT && dest == rname + 1 && *dest == '/')
        dest++;
    *dest = '\0';
    if(rname_limit != dest + 1)
        rname = xrealloc(rname, dest - rname + 1);

    free(extra_buf);
    if(ht)
        hash_free(ht);
    return rname;

error:
    free(extra_buf);
    free(rname);
    if(ht)
        hash_free(ht);
    errno = saved_errno;
    return NULL;
}
