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
#define xph_filez   (size_t)((clazz == ELFCLASS32       \
                    ? elf_getu32(swap, ph32.p_filesz)   \
                    : elf_getu64(swap, ph64.p_filesz)))
#define xnh_addr    (clazz == ELFCLASS32        \
                    ? (void*)&nh32              \
                    : (void*)&nh64)
#define xnh_sizeof  (clazz == ELFCLASS32        \
                    ? sizeof nh32               \
                    : sizeof nh64)
#define xnh_namesz  (clazz == ELFCLASS32        \
                    ? elf_getu32(swap, nh32.n_namesz)    \
                    : elf_getu32(swap, nh64.n_namesz))
#define xnh_descsz  (clazz == ELFCLASS32        \
                    ? elf_getu32(swap, nh32.n_descsz)   \
                    : elf_getu32(swap, nh64.n_descsz))
#define xnh_type    (clazz == ELFCLASS32        \
                    ? elf_getu32(swap, nh32.n_type) \
                    : elf_getu32(swap, nh64.n_type))


#define FLAGS_DID_CORE          0x01
#define FLAGS_DID_NOTE          0x02
#define FLAGS_DID_BUILD_ID      0x04
#define FLAGS_DID_CORE_TYPLE    0x08
#define FLAGS_IS_CORE           0x10


#define ELF_ALIGN(a)        ((((a) + align - 1) / align) * align)


static size_t
donote(struct magic_set* ms, void* vbuf, size_t offset, size_t size,
        int clazz, int swap, size_t align, int* flags)
{
    Elf32_Nhdr nh32;
    Elf64_Nhdr nh64;
    size_t noff, doff;
    int os_style = -1;

    uint32_t namesz, descsz;
    unsigned char* nbuf = CAST(unsigned char*, vbuf);

    memcpy(xnh_addr, &nbuf[offset], xnh_sizeof);
    offset += xnh_sizeof;

    namesz = xnh_namesz;
    descsz = xnh_descsz;
    if((namesz == 0) && (descsz == 0))
    {
        /* We're out of note headers */
        return (offset >= size) ? offset : size;
    }

    if(namesz & 0x80000000)
    {
        file_printf(ms, ", bad note name size 0x%lx",
                    (unsigned long)namesz);
        return offset;
    }

    if(descsz & 0x80000000)
    {
        file_printf(ms, ", bad note description size 0x%lx",
                    (unsigned long)descsz);
        return offset;
    }

    noff = offset;
    doff = ELF_ALIGN(offset + namesz);

    if(offset + namesz > size)
    {
        /* We're past the end of the buffer */
        return doff;
    }

    offset = ELF_ALIGN(doff + descsz);
    if(doff + descsz > size)
    {
        /* We're past the end of the buffer */
        return (offset >= size) ? offset : size;
    }

    if((*flags & (FLAGS_DID_NOTE | FLAGS_DID_BUILD_ID)) ==
            (FLAGS_DID_NOTE | FLAGS_DID_BUILD_ID))
        goto core;

    if(namesz == 5 && strcmp((char*)&nbuf[noff], "SuSE") == 0 &&
            xnh_type == NT_GNU_VERSION && descsz == 2)
    {
        file_printf(ms, ", for SuSE %d.%d", nbuf[doff], nbuf[doff + 1]);
    }

    if(namesz == 4 && strcmp((char*)&nbuf[noff], "GNU") == 0 &&
            xnh_type == NT_GNU_VERSION && descsz == 16)
    {
        uint32_t desc[4];
        memcpy(desc, &nbuf[doff], sizeof(desc));

        if(file_printf(ms, ", for GNU/") == -1)
            return size;
        switch(elf_getu32(swap, desc[0]))
        {
            case GNU_OS_LINUX:
                if(file_printf(ms, "Linux") == -1)
                    return size;
                break;
            case GNU_OS_HURD:
                if(file_printf(ms, "Hurd") == -1)
                    return size;
                break;
            case GNU_OS_SOLARIS:
                if(file_printf(ms, "Solaris") == -1)
                    return size;
                break;
            case GNU_OS_KFREEBSD:
                if(file_printf(ms, "kFreeBSD") == -1)
                    return size;
                break;
            case GNU_OS_KNETBSD:
                if(file_printf(ms, "kNetBSD") == -1)
                    return size;
                break;
            default:
                if(file_printf(ms, "<unknown>") == -1)
                    return size;
        }
        if(file_printf(ms, " %d.%d.%d", elf_getu32(swap, desc[1]),
                elf_getu32(swap, desc[2]), elf_getu32(swap, desc[3])) == -1)
            return size;
        *flags |= FLAGS_DID_NOTE;
        return size;
    }

    if(namesz == 4 && strcmp((char*)&nbuf[noff], "GNU") == 0 &&
            xnh_type == NT_GNU_BUILD_ID && (descsz == 16 || descsz == 20))
    {
        uint8_t desc[20];
        uint32_t i;
        if(file_printf(ms, ", BuildID[%s]=", descsz == 16 ? "md5/uuid"
                                             : "sha1") == -1)
            return size;
        memcpy(desc, &nbuf[doff], descsz);
        for(i = 0; i < descsz; i++)
            if(file_printf(ms, "%02x", desc[i]) == -1)
                return size;
        *flags |= FLAGS_DID_BUILD_ID;
    }

    if(namesz == 7 && strcmp((char*)&nbuf[noff], "NetBSD") == 0 &&
            xnh_type == NT_NETBSD_VERSION && descsz == 4)
    {
        uint32_t desc;
        memcpy(&desc, &nbuf[doff], sizeof(desc));
        desc = elf_getu32(swap, desc);

        if(file_printf(ms, ", for NetBSD") == -1)
            return size;

        /* The version number used to be stuck as 199905, and was thus
           basically content-free. Newer version of NetBSD have fixed
           this and now use the encoding of __NetBSD_Version__:

            MMmmrrpp00

           M = major version
           m = minor version
           r = release ["",A-Z,Z[A-Z] but numeric]
           p = patchlevel
         */
        if(desc > 100000000U)
        {
            uint32_t ver_patch = (desc / 100) % 100;
            uint32_t ver_rel = (desc / 10000) % 100;
            uint32_t ver_min = (desc / 1000000) % 100;
            uint32_t ver_maj = desc / 100000000;

            if(file_printf(ms, " %u.%u", ver_maj, ver_min) == -1)
                return size;
            if(ver_rel == 0 && ver_patch != 0)
            {
                if(file_printf(ms, ".%u", ver_patch) == -1)
                    return size;
            } else if(ver_rel != 0)
            {
                while(ver_rel > 26)
                {
                    if(file_printf(ms, "Z") == -1)
                        return size;
                    ver_rel -= 26;
                }
                if(file_printf(ms, "%c", 'A' + ver_rel -1) == -1)
                    return size;
            }
        }
        *flags |= FLAGS_DID_NOTE;
        return size;
    }

    if(namesz == 8 && strcmp((char*)&nbuf[noff], "FreeBSD") == 0 &&
            xnh_type == NT_FREEBSD_VERSION && descsz == 4)
    {
        uint32_t desc;
        memcpy(&desc, &nbuf[doff], sizeof(desc));
        desc = elf_getu32(swap, desc);
        if(file_printf(ms, ", for FreeBSD") == -1)
            return size;

        /* Contents is __FreeBSD_version, whose relation to OS
           versions is defined by a huge table in the Porter's
           Handbook. This is the general scheme:

           Releases:
            Mmp000  (before 4.10)
            Mmi0p0  (before 5.0)
            Mmm0po  

           Development branches:
            Mmpxxx  (before 4.6)
            Mmp1xx  (before 4.10)
            Mmi1xx  (before 5.0)
            M000xx  (pre-M.0)
            Mmm1xx

          M = major version
          m = minor version
          i = minor version increment (491000 -> 4.10)
          p = patchlevel
          x = revision

          The first release of FreeBSD to use ELF by default
          was version 3.0 */
       if(desc == 460002)
       {
           if(file_printf(ms, " 4.6.2") == -1)
               return size;
       } else if(desc < 460100)
       {
           if(file_printf(ms, " %d.%d", desc / 100000,
                    (desc / 10000) % 10) == -1)
               return size;
           if(desc / 1000 % 10 > 0)
               if(file_printf(ms, ".%d", desc / 1000 % 10) == -1)
                   return size;
           if((desc % 1000 > 0) || (desc % 100000 == 0))
               if(file_printf(ms, " (%d)", desc) == -1)
                   return size;
       } else if(desc < 500000)
       {
           if(file_printf(ms, " %d.%d", desc / 100000,
                    desc / 10000 % 10 + desc / 1000 % 10) == -1)
               return size;
           if(desc / 100 % 10 > 0)
           {
               if(file_printf(ms, " (%d)", desc) == -1)
                   return size;
           } else if(desc / 10 % 10 > 0)
           {
               if(file_printf(ms, ".%d", desc / 10 % 10) == -1)
                   return size;
           }
       } else
       {
           if(file_printf(ms, " %d.%d", desc / 100000,
                    desc / 1000 % 100) == -1)
               return size;
           if((desc / 100 % 10 > 0) || (desc % 100000 / 100 == 0))
           {
               if(file_printf(ms, " (%d)", desc) == -1)
                   return size;
           } else if(desc / 10 % 10 > 0)
           {
               if(file_printf(ms, ".%d", desc / 10 % 10) == -1)
                   return size;
           }
       }
       *flags |= FLAGS_DID_NOTE;
       return size;
    }

    if(namesz == 8 && strcmp((char*)&nbuf[noff], "OpenBSD") == 0 &&
            xnh_type == NT_OPENBSD_VERSION && descsz == 4)
    {
        if(file_printf(ms, ", for OpenBSD") == -1)
            return size;
        /* Content of note is always 0 */
        *flags |= FLAGS_DID_NOTE;
        return size;
    }

    if(namesz == 10 && strcmp((char*)&nbuf[noff], "DragonFly") == 0 &&
            xnh_type == NT_DRAGONFLY_VERSION && descsz == 4)
    {
        uint32_t desc;
        if(file_printf(ms, ", for DragonFly") == -1)
            return size;
        memcpy(&desc, &nbuf[doff], sizeof(desc));
        desc = elf_getu32(swap, desc);
        if(file_printf(ms, " %d.%d.%d", desc / 100000,
                desc / 10000 % 10, desc / 10000) == -1)
            return size;
        *flags |= FLAGS_DID_NOTE;
        return size;
    }
core:
    /* The 2.0.36 kernel in Debian 2.1, at
       least, doesn't correctly implement name
       sections, in core dumps, as specified by
       the "Programming Linking" section of "UNIX(R) System
       V Release 4 Programmer's Guide: ANSI C and
       Programming Support Tools", because my copy
       clearly says "The first 'namesz' bytes in 'name'
       contain a *null-terminated* [emphasis mime]
       character representation of the entry's owner
       or originator", but the 2.0.36 kernel code
       doesn't include the terminating null in the
       name.... */
    if((namesz == 4 && strncmp((char*)&nbuf[noff], "CORE", 4) == 0) ||
        (namesz == 5 && strcmp((char*)&nbuf[noff], "CORE") == 0))
    {
        os_style = OS_STYLE_SVR4;
    }

    if((namesz == 8 && strcmp((char*)&nbuf[noff], "FreeBSD") == 0))
    {
        os_style = OS_STYLE_FREEBSD;
    }

    if((namesz >= 11 && strncmp((char*)&nbuf[noff], "NetBSD-CORE", 11) == 0))
    {
        os_style = OS_STYLE_NETBSD;
    }

    if((*flags & FLAGS_DID_CORE) != 0)
        return size;

    if(os_style != -1 && (*flags & FLAGS_DID_CORE_STYLE) == 0)
    {
        if(file_printf(ms, ", %s-style", os_style_names[os_style]) == -1)
            return size;
        *flags |= FLAGS_DID_CORE_STYLE;
    }

    switch(os_style)
    {
        case OS_STYLE_NETBSD:
            if(xnh_type == NT_NETBSD_CORE_PROCINFO)
            {
                uint32_t signo;
                /* Extract the program name. It is at
                   offset 0x7c, and is up to 32-bytes,
                   including the terminating NUL. */
                if(file_printf(ms, ", from '%.31s'",
                        &nbuf[doff + 0x7c]) == -1)
                    return size;

                /* Extract the signal number. It is at
                   offset 0x08 */
                memcpy(&signo, &nbuf[doff + 0x08], sizeof(signo));
                if(file_printf(ms, " (signal %u)",
                        elf_getu32(swap, signo)) == -1)
                    return size;
                *flags |= FLAGS_DID_CORE;
                return size;
            }
            break;

        default:
            if(xnh_type == NT_PRPSINFO && *flags & FLAGS_IS_CORE)
            {
                size_t i, j;
                unsigned char c;
                /* Extract the program name. We assume
                   it to be 16 characters (that's what it
                   is in SunOS 5.x and Linux).

                   Unfortunately, it's at a different offset
                   in various OSes, so try multiple offsets.
                   If the characters aren't all printable,
                   reject it */
                for(i = 0; i < NOFFSETS; i++)
                {
                    unsigned char *cname, *cp;
                    size_t reloffset = prpsoffsets(i);
                    size_t noffset = doff + reloffset;
                    size_t k;
                    for(j = 0; j < 16; j++, noffset++, reloffset++)
                    {
                        /* Make sure we're not past
                           the end of the buffer; if 
                           we are, just give up */
                        if(noffset >= size)
                            goto tryanother;

                        /* Make sure we're not past
                           the end of the contents;
                           if we are, this obviously
                           isn't the right offset */
                        if(reloffset >= descsz)
                            goto tryanother;

                        c = nbuf[noffset];
                        if(c == '\0')
                        {
                            /* A '\0' at the beginning is
                               obviously wrong. Any other
                               '\0' means we're done */
                            if(j == 0)
                                goto tryanother;
                            else
                                break;
                        } else
                        {
                            /* A nonprintable character
                               is also wrong */
                            if(!isprint(c) || isquote(c))
                                goto tryanother;
                        }
                    }
                    /* Well, that worked */

                    /* Try next offsets, in case this match is
                       in the middle of a string */
                    for(k = i + 1; k < NOFFSETS; k++)
                    {
                        size_t no;
                        int adjust = 1;
                        if(prpsoffsets(k) >= prpsoffsets(i))
                            continue;
                        for(no = doff + prpsoffsets(k);
                            no < doff + prpsoffsets(i); no++)
                        {
                            adjust = adjust && isprint(nbuf[no]);
                        }
                        if(adjust)
                            i = k;
                    }

                    cname = (unsigned char*)&nbuf[doff + prpsoffsets(i)];
                    for(cp = cname; *cp && isprint(*cp); cp++)
                        continue;

                    /* Linux apparently appends a space at the end
                       of the command line: remove it */
                    while(cp > cname && isspace(cp[-1]))
                        cp--;
                    if(file_printf(ms, ", from '%.*s'",
                            (int)(cp - cname), cname) == -1)
                        return size;
                    *flags |= FLAGS_DID_CORE;
                    return size;

                tryanother:
                    ;
                }
            }
            break;
    }
    return offset;
}



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

