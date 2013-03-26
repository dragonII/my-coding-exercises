#include "file.h"
#include "magic_.h"

#include <stdarg.h>

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
