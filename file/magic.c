#include "magic_.h"

#include <stdio.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

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
    return files_ms_alloc(flags);
}

void magic_close(struct magic_set* ms)
{
    if(ms == NULL)
        return;
    file_ms_free(ms);
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


