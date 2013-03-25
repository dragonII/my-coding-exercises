#include "file.h"
#include "magic_.h"

void file_oomem(struct magic_set* ms, size_t len)
{
    file_error(ms, errno, "cannot allocate %" SIZE_T_FORMAT "u bytes",
                len);
}

