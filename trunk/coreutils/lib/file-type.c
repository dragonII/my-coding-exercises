/* Return a string describing the type of a file */

#include "file-type.h"

#include <gettext.h>
#ifndef _
#define _(text) gettext(text)
#endif

const char* file_type(struct stat* st)
{
    /* See POSIX 1003.1-2001 XCU Table 4-8 lines 17093-17107 for some of
       these formats.

       To keep diagnostics grammatical in English, the returned string
       must start with a consonant */

    if(S_ISREG(st->st_mode))
        return st->st_size == 0 ? _("regular empty file") : _("regular file");

    if(S_ISDIR(st->st_mode))
        return _("directory");

    if(S_ISBLK(st->st_mode))
        return _("block special file");

    if(S_ISCHR(st->st_mode))
        return _("character special file");

    if(S_ISFIFO(st->st_mode))
        return _("fifo");

    if(S_ISLNK(st->st_mode))
        return _("symbolic link");

    if(S_ISSOCK(st->st_mode))
        return _("socket");

    if(S_TYPEISMQ(st))
        return _("message queue");

    if(S_TYPEISSEM(st))
        return _("semaphore");

    if(S_TYPEISSHM(st))
        return _("shared memory object");

    return _("weird file");
}
