#include "file.h"
#include "magic_.h"

#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/* like printf, only we append to a buffer */
int file_vprintf(struct magic_set* ms, const char* fmt, va_list ap)
{
    int len;
    char *buf, *newstr;

    len = vasprintf(&buf, fmt, ap);
    if(len < 0)
        goto out;

    if(ms->o.buf != NULL)
    {
        len = asprintf(&newstr, "%s%s", ms->o.buf, buf);
        free(buf);
        if(len < 0)
            goto out;
        free(ms->o.buf);
        buf = newstr;
    }
    ms->o.buf = buf;
    return 0;
out:
    file_error(ms, errno, "vasprintf failed");
    return -1;
}


int file_printf(struct magic_set* ms, const char* f, ...)
{
    int rv;
    va_list ap;

    va_start(ap, f);
    rv = file_vprintf(ms, f, ap);
    va_end(ap);
    return rv;
}


/* error - print best error message possible */
static void
file_error_core(struct magic_set* ms, int error, const char* f, va_list va,
                size_t lineno)
{
    /* Only the first error is ok */
    if(ms->event_flags & EVENT_HAD_ERR)
        return;
    if(lineno != 0)
    {
        free(ms->o.buf);
        ms->o.buf = NULL;
        file_printf(ms, "line %" SIZE_T_FORMAT "u: ", lineno);
    }
    file_vprintf(ms, f, va);
    if(error > 0)
        file_printf(ms, " (%s)", strerror(error));
    ms->event_flags |= EVENT_HAD_ERR;
    ms->error = error;
}

void file_error(struct magic_set* ms, int error, const char* f, ...)
{
    va_list va;
    va_start(va, f);
    file_error_core(ms, error, f, va, 0);
    va_end(va);
}

void file_oomem(struct magic_set* ms, size_t len)
{
    file_error(ms, errno, "cannot allocate %" SIZE_T_FORMAT "u bytes",
                len);
}


int file_check_mem(struct magic_set* ms, unsigned int level)
{
    size_t len;

    if(level >= ms->c.len)
    {
        len = (ms->c.len += 20) * sizeof(*ms->c.li);
        ms->c.li = CAST(struct level_info*, (ms->c.li == NULL) ?
                        malloc(len) : realloc(ms->c.li, len));
        if(ms->c.li == NULL)
        {
            file_oomem(ms, len);
            return -1;
        }
    }
    ms->c.li[level].got_match = 0;
#ifdef ENABLE_CONDITIONALS
    ms->c.li[level].last_match = 0;
    ms->c.li[level].last_cond = COND_NONE;
#endif
    return 0;
}


/* Print an error with magic line number */
void file_magerror(struct magic_set* ms, const char* f, ...)
{
    va_list va;
    va_start(va, f);
    file_error_core(ms, 0, f, va, ms->line);
    va_end(va);
}


int file_reset(struct magic_set* ms)
{
    if(ms->mlist[0] == NULL)
    {
        file_error(ms, 0, "no magic files loaded");
        return -1;
    }

    if(ms->o.buf)
    {
        free(ms->o.buf);
        ms->o.buf = NULL;
    }

    if(ms->o.pbuf)
    {
        free(ms->o.pbuf);
        ms->o.pbuf = NULL;
    }

    ms->event_flags &= ~EVENT_HAD_ERR;
    ms->error = -1;
    return 0;
}

