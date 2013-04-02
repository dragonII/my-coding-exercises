#include "file_.h"
#include "readelf.h"
#include "magic_.h"

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


static uint16_t
getu16(int swap, uint16_t value)
{
    union
    {
        uint16_t ui;
        char c[2];
    } retval, tmpval;

    if(swap)
    {
        tmpval.ui = value;

        retval.c[0] = tmpval.c[1];
        retval.c[1] = tmpval.c[0];

        return retval.ui;
    } else
        return value;
}

static uint32_t
getu32(int swap, uint32_t value)
{
    union
    {
        uint32_t ui;
        char c[4];
    } retval, tmpval;

    if(swap)
    {
        tmpval.ui = value;

        retval.c[0] = tmpval.c[3];
        retval.c[1] = tmpval.c[2];
        retval.c[2] = tmpval.c[1];
        retval.c[3] = tmpval.c[0];

        return retval.ui;
    } else
        return value;
}

static uint64_t 
getu64(int swap, uint64_t value)
{
    union
    {
        uint64_t ui;
        char c[8];
    } retval, tmpval;

    if(swap)
    {
        tmpval.ui = value;

        retval.c[0] = tmpval.c[7];
        retval.c[1] = tmpval.c[6];
        retval.c[2] = tmpval.c[5];
        retval.c[3] = tmpval.c[4];
        retval.c[4] = tmpval.c[3];
        retval.c[5] = tmpval.c[2];
        retval.c[6] = tmpval.c[1];
        retval.c[7] = tmpval.c[0];

        return retval.ui;
    } else
        return value;
}



#define elf_getu16(swap, value) getu16(swap, value)
#define elf_getu32(swap, value) getu32(swap, value)
#define elf_getu64(swap, value) getu64(swap, value)


#define xph_addr    (clazz == ELFCLASS32 ? (void*) &ph32 : (void*) &ph64)
#define xph_sizeof  (clazz == ELFCLASS32 ? sizeof(ph32) : sizeof(ph64))
#define xph_type    (clazz == ELFCLASS32        \
                    ? elf_getu32(swap, ph32.p_type)     \
                    : elf_getu32(swap, ph64.p_type))
#define xph_offset  (off_t)(clazz == ELFCLASS32 \
                    ? elf_getu32(swap, ph32.p_offset)   \
                    : elf_getu64(swap, ph64.p_offset))


#define FLAGS_DID_CORE          0x01
#define FLAGS_DID_NOTE          0x02
#define FLAGS_DID_BUILD_ID      0x04
#define FLAGS_DID_CORE_TYPLE    0x08
#define FLAGS_IS_CORE           0x10

static int
dophn_core(struct magic_set* ms, int clazz, int swap, int fd, off_t off,
            int num, size_t size, off_t fsize, int* flags)
{
    Elf32_Phdr  ph32;
    Elf64_Phdr  ph64;
    size_t offset, len;
    unsigned char nbuf[BUFSIZ];
    ssize_t bufsize;

    if(size != xph_sizeof)
    {
        if(file_printf(ms, ", corrputed program header size") == -1)
            return -1;
        return 0;
    }

    /* Loop through all the program headers */
    for(; num; num--)
    {
        if(pread(fd, xph_addr, xph_sizeof, off) == -1)
        {
            file_badread(ms);
            return -1;
        }
        off += size;

        if(xph_offset > fsize)
        {
            /* Perhaps warn here */
            continue;
        }

        if(xph_type != PT_NOTE)
            continue;

        /* This is a PT_NOTE section; loop through all the notes
           in the section */
        len = xph_filez < sizeof(nbuf) ? xph_filez : sizeof(nbuf);
        if((bufsize = pread(fd, nbuf, len, xph_offset)) == -1)
        {
            file_badread(ms);
            return -1;
        }
        offset = 0;
        for(;;)
        {
            if(offset >= (size_t)bufsize)
                break;
            offset = donote(ms, nbuf, offset, (size_t)bufsize,
                            clazz, swap, 4, flags);
            if(offset == 0)
                break;
        }
    }
    return 0;
}

int file_tryelf(struct magic_set* ms, int fd, const unsigned char* buf,
                size_t nbytes)
{
    union
    {
        int32_t l;
        char c[sizeof(int32_t)];
    } u;
    int clazz;
    int swap;
    struct stat st;
    off_t fsize;
    int flags = 0;
    Elf32_Ehdr elf32hdr;
    Elf64_Ehdr elf64hdr;
    uint16_t type;

    if(ms->flags & (MAGIC_MIME | MAGIC_APPLE))
        return 0;

    /* ELF executables have mutiple section headers in arbitrary
       file locations and thus file(1) cannot determine it from easily.
       Instead we traverse thru all section headers until a symbo table
       one is found or else the binary is stripped.
       Return immediately if it's no ELF (so we avoid pipe2file unless needed)
       */
    if(buf[EI_MAG0] != ELFMAG0
        || (buf[EI_MAG1] != ELFMAG1 && buf[EI_MAG1] != OLFMAG1)
        || buf[EI_MAG2] != ELFMAG2 || buf[EI_MAG3] != ELFMAG3)
        return 0;

    /* If we cannot seek, it must be a pipe, socket or fifo */
    if((lseek(fd, (off_t)0, SEEK_SET) == (off_t)-1) && (errno == ESPIPE))
        fd = file_pipe2file(ms, fd, buf, nbytes);

    if(fstat(fd, &st) == -1)
    {
        file_badread(ms);
        return -1;
    }
    fsize = st.st_size;
    clazz = buf[EI_CLASS];

    switch(clazz)
    {
        case ELFCLASS32:
#undef elf_getu
#define elf_getu(a, b) elf_getu32(a, b)
#undef elfhdr
#define elfhdr elf32hdr
#include "elfclass.h"
        case ELFCLASS64:
#undef elf_getu
#define elf_getu(a, b) elf_getu64(a, b)
#undef elfhdr
#define elfhdr elf64hdr
#include "elfclass.h"
        default:
            if(file_printf(ms, ", unknown class %d", clazz) == -1)
                return -1;
            break;
    }
    return 0;
}

