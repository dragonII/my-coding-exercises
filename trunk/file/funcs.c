#include "file_.h"
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


int file_buffer(struct magic_set* ms, int fd, const char* inname __attribute__((unused)), const void* buf, size_t nb)
{
    int m = 0, rv = 0, looks_text = 0;
    int mime = ms->flags & MAGIC_MIME;
    const unsigned char* ubuf = CAST(const unsigned char*, buf);
    unichar* u8buf = NULL;
    size_t ulen;
    const char* code = NULL;
    const char* code_mime = "binary";
    const char* type = NULL;

    if(nb == 0)
    {
        if((!mime || (mime & MAGIC_MIME_TYPE)) &&
            file_printf(ms, mime ? "application/x-empty"
                                 : "empty") == -1)
        {
            return -1;
        }
        return -1;
    } else if(nb == 1)
    {
        if((!mime || (mime & MAGIC_MIME_TYPE)) &&
            file_printf(ms, mime ? "application/octet-stream" 
                                 : "very short file (no magic)") == -1)
        {
            return -1;
        }
        return -1;
    }

    if((ms->flags & MAGIC_NO_CHECK_ENCODING) == 0)
    {
        looks_text = file_encoding(ms, ubuf, nb, &u8buf, &ulen,
                        &code, &code_mime, &type);
    }

    /* try compression stuff */
    if((ms->flags & MAGIC_NO_CHECK_COMPRESS) == 0)
        if((m = file_zmagic(ms, fd, inname, ubuf, nb)) != 0)
        {
            if((ms->flags & MAGIC_DEBUG) != 0)
                (void)fprintf(stderr, "zmagic %d\n", m);
            goto done;
        }

    /* check if we have a tar file */
    if((ms->flags & MAGIC_NO_CHECK_TAR) == 0)
        if((m = file_is_tar(ms, ubuf, nb)) != 0)
        {
            if((ms->flags & MAGIC_DEBUG) != 0)
                (void)fprintf(stderr, "tar %d\n", m);
            goto done;
        }

    /* check if we have CDF file */
    if((ms->flags & MAGIC_NO_CHECK_CDF) == 0)
        if((m = file_trycdf(ms, fd, ubuf, nb)) != 0)
        {
            if((ms->flags & MAGIC_DEBUG) != 0)
                (void)fprintf(stderr, "cdf %d\n", m);
            goto done;
        }

    /* try soft magic tests */
    if((ms->flags & MAGIC_NO_CHECK_SOFT) == 0)
        if((m = file_softmagic(ms, ubuf, nb, BINTEST, looks_text)) != 0)
        {
            if((ms->flags & MAGIC_DEBUG) != 0)
                (void)fprintf(stderr, "softmagic %d\n", m);

            if((ms->flags & MAGIC_NO_CHECK_ELF) == 0 && m == 1 &&
                    nb > 5 && fd != -1)
            {
                /* we matched something in the file, so this
                   *might* be an ELF file, and the file is at
                   least 5 bytes long, so if it's an ELF file
                   it has at least one byte past the ELF magic
                   number - try extracting information from the
                   ELF headers that cannot easilly * be
                   extracted with rules in the magic file. */
                if((m = file_tryelf(ms, fd, ubuf, nb)) != 0)
                    if((ms->flags & MAGIC_DEBUG) != 0)
                        (void)fprintf(stderr, "elf %d\n", m);
            }
            goto done;
        }

    /* try text properties */
    if((ms->flags & MAGIC_NO_CHECK_TEXT) == 0)
    {
        if((m = file_ascmagic(ms, ubuf, nb, looks_text)) != 0)
        {
            if((ms->flags & MAGIC_DEBUG) != 0)
                (void)fprintf(stderr, "ascmagic %d\n", m);
            goto done;
        }

        /* try to discover text encoding */
        if((ms->flags & MAGIC_NO_CHECK_ENCODING) == 0)
        {
            if(looks_text == 0)
            {
                if((m = file_ascmagic_with_encoding(ms, ubuf,
                            nb, u8buf, ulen, code, type, looks_text)) != 0)
                {
                    if((ms->flags & MAGIC_DEBUG) != 0)
                        (void)fprintf(stderr, "ascmagic/enc %d\n", m);
                    goto done;
                }
            }
        }
    }

    /* give up */
    m = 1;
    if((!mime || (mime & MAGIC_MIME_TYPE)) &&
            file_printf(ms, mime ? "application/octet-stream" : "data") == -1)
    {
        rv = -1;
    }

done:
    if((ms->flags & MAGIC_MIME_ENCODING) != 0)
    {
        if(ms->flags & MAGIC_MIME_TYPE)
            if(file_printf(ms, "; charset=") == -1)
                rv = -1;
        if(file_printf(ms, "%s", code_mime) == -1)
            rv = -1;
    }
    free(u8buf);
    if(rv)
        return rv;

    return m;
}




