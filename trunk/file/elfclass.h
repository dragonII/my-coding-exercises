    if(nbytes <= sizeof(elfhdr))
        return 0;

    u.l = 1;
    memcpy(&elfhdr, buf, sizeof elfhdr);
    swap = (u.c[sizeof(int32_t) - 1] + 1) != elfhdr.e_ident[EI_DATA];

    type = elf_getu16(swap, elfhdr.e_type);

    switch(type)
    {
        case ET_CORE:
            flags |= FLAGS_IS_CORE;
            if(dophn_core(ms, clazz, swap, fd,
                        (off_t)elf_getu(swap, elfhdr.e_phoff),
                        elf_getu16(swap, elfhdr.e_phnum),
                        (size_t)elf_getu16(swap, elfhdr.e_phentsize),
                        fsize, &flags) == -1)
                return -1;
            break;

        case ET_EXEC:
        case ET_DYN:
            if(dophn_exec(ms, clazz, swap, fd,
                        (off_t)elf_getu(swap, elfhdr.e_phoff),
                        elf_getu16(swap, elfhdr.e_phnum),
                        (size_t)elf_getu16(swap, elfhdr.e_phentsize),
                        fsize, &flags, elf_getu16(swap, elfhdr.e_shnum))
                        == -1)
                return -1;
            /* FALLTHROUGH */
        case ET_REL:
            if(doshn(ms, clazz, swap, fd,
                        (off_t)elf_getu(swap, elfhdr.e_shoff),
                        elf_getu16(swap, elfhdr.e_shnum),
                        (size_t)elf_getu16(swap, elfhdr.e_shentsize),
                        fsize, &flags, elf_getu16(swap, elfhdr.e_machine),
                        (int)elf_getu16(swap, elfhdr.e_shstrndx)) == -1)
                return -1;
            break;

        default:
            break;
    }

    return 1;


