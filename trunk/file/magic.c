#include "magic_.h"

#include <stdio.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

static const char*
get_default_magic(void)
{
    static const char hmagic[] = "/.magic/magic.mgc";
    static char* default_magic;
    char *home, *hmagicpath;

    struct stat st;

    if(default_magic)
    {
        free(default_magic);
        default_magic = NULL;
    }

    if((home = getenv("HOME")) == NULL)
        return MAGIC;

    if(asprintf(&hmagicpath, "%s/.magic.mgc", home) < 0)
        return MAGIC;
    if(stat(hmagicpath, &st) == -1)
    {
        free(hmagicpath);
        if(asprintf(&hmagicpath, "%s/.magic", home) < 0)
            return MAGIC;
        if(stat(hmagicpath, &st) == -1)
            goto out;
        if(S_ISDIR(st.st_mode))
        {
            free(hmagicpath);
            if(asprintf(&hmagicpath, "%s/%s", home, hmagic) < 0)
                return MAGIC;
            if(access(hmagicpath, R_OK) == -1)
                goto out;
        }
    }

    if(asprintf(&default_magic, "%s:%s", hmagicpath, MAGIC) < 0)
        goto out;
    free(hmagicpath);
    return default_magic;
out:
    default_magic = NULL;
    free(hmagicpath);
    return MAGIC;
}

int magic_version(void)
{
    return MAGIC_VERSION;
}

int magic_compile(struct magic_set* ms, const char* magicfile)
{
    if(ms == NULL)
        return -1;
    return file_apprentice(ms, magicfile, FILE_COMPILE);
}

int magic_list(struct magic_set* ms, const char* magicfile)
{
    if(ms == NULL)
        return -1;
    return file_apprentice(ms, magicfile, FILE_LIST);
}

const char*
magic_error(struct magic_set* ms)
{
    if(ms == NULL)
        return "Magic database is not open";
    return (ms->event_flags & EVENT_HAD_ERR) ? ms->o.buf : NULL;
}

const char*
magic_getpath(const char* magicfile, int action)
{
    if(magicfile != NULL)
        return magicfile;

    magicfile = getenv("MAGIC");
    if(magicfile != NULL)
        return magicfile;

    return action == FILE_LOAD ? get_default_magic() : MAGIC;
}

struct magic_set*
magic_open(int flags)
{
    return file_ms_alloc(flags);
}

void magic_close(struct magic_set* ms)
{
    if(ms == NULL)
        return;
    file_ms_free(ms);
}


#ifndef COMPILE_ONLY
static const char*
file_or_fd(struct magic_set* ms, const char* inname, int fd)
{
    int rv = -1;
    unsigned char* buf;
    struct stat sb;
    ssize_t nbytes = 0;     /* number of bytes read from a datafile */
    int ispipe = 0;

    /* one extra for terminating '\0' and
       some overlapping space for matches near EOF */
#define SLOP (1 + sizeof(union VALUETYPE))
    if((buf = CAST(unsigned char*, malloc(HOWMANY + SLOP))) == NULL)
        return NULL;

    if(file_reset(ms) == -1)
        goto done;

    switch(file_fsmagic(ms, inname, &sb))
    {
        case -1:    /* error */
            goto done;
        case 0:     /* nothing found */
            break;
        default:    /* matched it and printed type */
            rv = 0;
            goto done;
    }

    if(inname == NULL)
    {
        if(fstat(fd, &sb) == 0 && S_ISFIFO(sb.st_mode))
            ispipe = 1;
    } else
    {
        int flags = O_RDONLY | O_BINARY;

        if(stat(inname, &sb) == 0 && S_ISFIFO(sb.st_mode))
        {
#ifdef O_NONBLOCK
            flags |= O_NONBLOCK;
#endif
            ispipe = 1;
        }

        errno = 0;
        if((fd = open(inname, flags)) < 0)
        {
            if(unreadable_info(ms, sb.st_mode, inname) == -1)
                goto done;
            rv = 0;
            goto done;
        }

#ifdef O_NONBLOCK
        if((flags = fcntl(fd, F_GETFL)) != -1)
        {
            flags &= ~O_NONBLOCK;
            (void)fcntl(fd, F_SETFL, flags);
        }
#endif
    }

    /* try looking at the first HOWMANY bytes */
    if(ispipe)
    {
        ssize_t r = 0;

        while((r = sread(fd, (void*)&buf[nbytes],
                (size_t)(HOWMANY - nbytes), 1)) > 0)
        {
            nbytes += r;
            if(r < PIPE_BUF) break;
        }

        if(nbytes == 0)
        {
            /* We cannot read it, but we were able to stat it */
            if(unreadable_info(ms, sb.st_mode, inname) == -1)
                goto done;
            rv = 0;
            goto done;
        }
    } else
    {
        if((nbytes = read(fd, (char*)buf, HOWMANY)) == -1)
        {
            file_error(ms, errno, "cannot read `%s'", inname);
            goto done;
        }
    }

    (void)memset(buf + nbytes, 0, SLOP);    /* NULL terminated */
    if(file_buffer(ms, fd, inname, buf, (size_t)nbytes) == -1)
        goto done;
    rv = 0;
done:
    free(buf);
    close_and_restore(ms, inname, fd, &sb);
    return rv == 0 ? file_getbuffer(ms) : NULL;
}



/* find type of named file */
const char*
magic_file(struct magic_set* ms, const char* inname)
{
    if(ms == NULL)
        return NULL;
    return file_or_fd(ms, inname, STDIN_FILENO);
}


/* load a magic file */
int magic_load(struct magic_set* ms, const char* magicfile)
{
    if(ms == NULL)
        return -1;
    return file_apprentice(ms, magicfile, FILE_LOAD);
}

int magic_check(struct magic_set* ms, const char* magicfile)
{
    if(ms == NULL)
        return -1;
    return file_apprentice(ms, magicfile, FILE_CHECK);
}

#endif
