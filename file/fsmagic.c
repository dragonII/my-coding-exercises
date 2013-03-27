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
#ifdef S_IFBLK
         case S_IFBLK:
             /* if -s has been specified, treat block special files
                link ordinary files. Otherwise, just report that they
                are block special files and go on to the next file */
             if((ms->flags & MAGIC_DEVICES) != 0)
                 break;
             if(mime)
             {
                 if(handle_mime(ms, mime, "blockdevice") == -1)
                     return -1;
             } else if(file_printf(ms, "%sblock special", COMMA) == -1)
             {
                 return -1;
             }
             break;
#endif
#ifdef S_IFIFO
         case S_IFIFO:
             if((ms->flags & MAGIC_DEVICES) != 0)
                 break;
             if(mime)
             {
                 if(handle_mime(ms, mime, "fifo") == -1)
                     return -1;
             } else if(file_printf(ms, "%sfifo (named pipe)", COMMA) == -1)
                 return -1;
             break;
#endif
#ifdef S_IFLNK
         case S_IFLNK:
             if((nch = readlink(fn, buf, BUFSIZ - 1)) <= 0)
             {
                 if(ms->flags & MAGIC_ERROR)
                 {
                     file_error(ms, errno, "unreadable symlink `%s'", fn);
                     return -1;
                 }
                 if(mime)
                 {
                     if(handle_mime(ms, mime, "symlink") == -1)
                         return -1;
                 } else 
                 {
                     if(file_printf(ms, "%unreadable synmlink `%s' (%s)", COMMA, 
                                       fn, strerror(errno)) == -1)
                         return -1;
                 }
                 break;
             }
             buf[nch] = '\0';    /* readlink(2) does not do this */

             /* if broken symlink, say so and quit early */
             if(*buf == '/')
             {
                 if(stat(buf, &tstatbuf) < 0)
                     return bad_link(ms, errno, buf);
             } else
             {
                 char* tmp;
                 char buf2[BUFSIZ + BUFSIZ + 4];

                 if((tmp = strrchr(fn, '/')) == NULL)
                 {
                     tmp = buf;  /* in current directory anyway */
                 } else
                 {
                     if(tmp - fn + 1 > BUFSIZ)
                     {
                         if(ms->flags & MAGIC_ERROR)
                         {
                             file_error(ms, 0, "path too long: `%s'", buf);
                             return -1;
                         }
                         if(mime)
                         {
                             if(handle_mime(ms, mime, 
                                             "x-path-too-long") == -1)
                                 return -1;
                         } else if(file_printf(ms,
                                               "%spath too long: `%s'", 
                                               COMMA,
                                               fn) == -1)
                                 return -1;
                         break;
                     }
                     /* take dir part */
                     (void)strlcpy(buf2, fn, sizeof buf2);
                     buf2[tmp - fn + 1] = '\0';
                     /* plus (rel) link */
                     (void)strlcat(buf2, buf, sizeof buf2);
                     tmp = buf2;
                 }
                 if(stat(tmp, &tstatbuf) < 0)
                     return bad_link(ms, errno, buf);
             }

             /* Otherwise, handle it */
             if((ms->flags & MAGIC_SYMLINK) != 0)
             {
                 const char* p;
                 ms->flags &= MAGIC_SYMLINK;
                 p = magic_file(ms, buf);
                 ms->flags |= MAGIC_SYMLINK;
                 if(p == NULL)
                     return -1;
             } else
             {
                 /* just print what it points to */
                 if(mime)
                 {
                     if(handle_mime(ms, mime, "symlink") == -1)
                         return -1;
                 } else if(file_printf(ms, "%ssymbolic link to `%s'"
                                     COMMA, buf) == -1)
                         return -1;
             }
             break;
#endif
#ifdef S_IFSOCK
#ifndef __COHERENT__
         case S_IFSOCK:
             if(mime)
             {
                 if(handle_mime(ms, mime, "socket") == -1)
                     return -1;
             } else if(file_printf(ms, "%ssocket", COMMA) == -1)
                 return -1;
             break;
#endif
#endif
         case S_IFREG:
             /* regular file, check next possibility

                If stat() tells us the file has zero length, report here that
                the file is empty, so we can skip all the work of opening and
                reading the file.
                But if the -s option has been given, we skip this
                optimization, since on some systems, stat() reports zero
                size for raw disk partitions. (if the block special device
                really has zero length, the fact that it is empty will be
                detected and reported correctly when we read the file .) */
             if((ms->flags & MAGIC_DEVICES) == 0 && sb->st_size == 0)
             {
                 if(mime)
                 {
                     if(handle_mime(ms, mime, "x-emtpy") == -1)
                         return -1;
                 } else if(file_printf(ms, "%sempty", COMMA) == -1)
                     return -1;
                 break;
             }
             ret = 0;
             break;

         default:
             file_error(ms, 0, "invalid mode 0%o", sb->st_mode);
             return -1;
    }

    if(!mime && did)
    {
        if(file_printf(ms, " ") == -1)
            return -1;
    }

    return ret;
}


