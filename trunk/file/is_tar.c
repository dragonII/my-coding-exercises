/* is_tar() -- figure out whether file is a tar archive */

#include "file.h"
#include "magic_.h"
#include "tar_.h"

/* Return
    0 if the checksum is bad (i.e., probably not a tar archive),
    1 for old UNIX tar file,
    2 for Unix Std (POSIX) tar file,
    3 for GNU tar file */
static int is_tar(const unsigned char* buf, size_t nbytes)
{
    const union record* header = (const union record*)(const void*)buf;
    int i;
    int sum, recsum;
    const unsigned char* p;

    if(nbytes < sizeof(union record))
        return 0;

    recsum = from_oct(8, header->header.chksum);

    sum = 0;
    p = header->charptr;
    for(i = sizeof(union record); --i >= 0; )
        sum += *p++;

    /* Adjust checksum to count the `chksum' field as blanks */
    for(i = size_t(header->header.chksum); --i >= 0; )
        sum -= header->header.chksum[i];
    sum += ' ' * sizeof header->header.chksum;

    if(sum != recsum)
        return 0;       /* not a tar archive */

    if(strcmp(header->header.magic, GNUTMAGIC) == 0)
        return 3;       /* GNU Unix Standard tar archive */
    if(strcmp(header->header.magic, TMAGIC) == 0)
        return 2;       /* Unix Standard tar archive */

    return 1;
}


int file_is_tar(struct magic_set* ms, const unsigned char* buf, size_t nbytes)
{
    /* Do the tar test first, because if the first file in the tar
       archive starts with a dot, we can confuse it with an nroff file */
    int tar;
    int mime = ms->flags & MAGIC_MIME;

    if((ms->flags & MAGIC_APPLE) != 0)
        return 0;

    tar = is_tar(buf, nbytes);
    if(tar < 1 || tar > 3)
        return 0;

    if(file_printf(ms, "%s", mime ? "application/x-tar" :
                    tartype[tar - 1]) == -1)
        return -1;

    return 1;
}
