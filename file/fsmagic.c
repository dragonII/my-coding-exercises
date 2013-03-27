/* fsmagic - magic based on filesystem info - directory, special files, etc */

#include "file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int file_fsmagic(struct magic_set* ms, const char* fn, struct stat* sb)
{
    int ret, did = 0;
    int mime = ms->flags & MAGIC_MIME;
#ifdef S_IFLNK
    char buf[BUFSIZ + 4];
    ssize_t nch;
    struct stat tstatbuf;
#endif

    if(ms->flags & MAGIC_APPLE)
        return 0;
    if(fn == NULL)
        return 0;

#define COMMA (did++ ? ", " : "")

    /* fstat is cheaper but fails for files you don't have read perms on.
       On 4.2BSD and similar systems, use lstat() to identify symlinks */
#ifdef S_IFLNK
       if((ms->flags & MAGIC_SYMLINK) == 0)
           ret = lstat(fn, sb);
       else
#endif
       ret = stat(fn, sb);      /* don't merge into if; see "ret =" above */

       if(ret)
       {
           if(ms->flags & MAGIC_ERROR)
           {
               file_error(ms, errno, "cannot stat `%s'", fn);
               return -1;
           }
           if(file_printf(ms, "cannot open `%s' (%s)",
               fn, strerror(errno)) == -1)
               return -1;
           ms->event_flags |= EVENT_HAD_ERR;
           return -1;
       }

       ret = -1;
       if(!mime)
       {
#ifdef S_ISUID
           if(sb->st_mode & S_ISUID)
               if(file_printf(ms, "%ssetuid", COMMA) == -1)
                   return -1;
#endif
#ifdef S_ISGID
           if(sb->st_mode & S_ISGID)
               if(file_printf(ms, "%ssetgid", COMMA) == -1)
                   return -1;
#endif
#ifdef S_ISVTX
           if(sb->st_mode & S_ISVTX)
               if(file_printf(ms, "%ssticky", COMMA) == -1)
                   return -1;
#endif
       }

       switch(sb->st_mode & S_IFMT)
       {
           case S_IFDIR:
            if(mime)
            {
                if(handle_mime(ms, mime, "directory") == -1)
                    return -1;
            } else if(file_printf(ms, "%sdirectory", COMMA) == -1)
                return -1;
            break;

#ifdef S_IFCHR:
            case S_IFCHR:
                /* if -s has been specified, treat character special files
                   like ordinary files. Otherwise, just report that they
                   are block special files and go on to the next file */
                if((ms->flags & MAGIC_DEVICES) != 0)
                    break;
                if(mime)
                {
                    if(handle_mime(ms, mime, "chardevice") == -1)
                        return -1;
                } else if(file_printf(ms, "%scharacter special", COMMA) == -1)
                {
                    return -1;
                }
                break;
#endif
       
