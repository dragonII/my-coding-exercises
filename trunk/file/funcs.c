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


