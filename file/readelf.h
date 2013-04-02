#ifndef __fake_elf_h__
#define __fake_elf_h__

#include <stdint.h>

typedef uint32_t Elf32_Char;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Word;
typedef uint32_t Elf32_Addr;

#define SIZE_LONG_LONG 8

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint64_t Elf64_Xword;
typedef uint16_t Elf64_Half;
typedef uint16_t Elf64_Word;
typedef uint8_t  Elf64_Char;


#define EI_NIDENT   16

typedef struct
{
    Elf32_Char  e_ident[EI_NIDENT];
    Elf32_Half  e_type;
    Elf32_Half  e_machine;
    Elf32_Word  e_version;
    Elf32_Addr  e_entry;        /* Entry point */
    Elf32_Off   e_phoff;
    Elf32_Off   e_shoff;
    Elf32_Word  e_flags;
    Elf32_Half  e_ehsize;
    Elf32_Half  e_phentsize;
    Elf32_Half  e_phnum;
    Elf32_Half  e_shentsize;
    Elf32_Half  e_shnum;
    Elf32_Half  shstrndx;
} Elf32_Ehdr;


typedef struct 
{
    Elf64_Char  e_ident[EI_NIDENT];
    Elf64_Half  e_type;
    Elf64_Half  e_machine;
    Elf64_Word  e_version;
    Elf64_Addr  e_entry;        /* Entry point */
    Elf64_Off   e_phoff;
    Elf64_Off   e_shoff;
    Elf64_Word  e_flags;
    Elf64_Half  e_ehsize;
    Elf64_Half  e_phentsize;
    Elf64_Half  e_phnum;
    Elf64_Half  e_shentsize;
    Elf64_Half  e_shnum;
    Elf64_Half  shstrndx;
} Elf64_Ehdr;


/* e_type */
#define ET_REL      1
#define ET_EXEC     2
#define ET_DYN      3
#define ET_CORE     4

/* e_machine (used only for SunOS 5.x hardware capabilities) */
#define EM_SPARC    2
#define EM_386      3
#define EM_SPARC32PLUS  18
#define EM_SPARCV9  43
#define EM_IA_64    50
#define EM_AMD64    62

/* sh_type */
#define SHT_SYMTAB  2
#define SHT_NOTE    7
#define SHT_DYNSYM  11
#define SHT_SUNW_cap    0x6ffffff5  /* SunOS 5.x hw/sw capabilities */

/* elf type */
#define ELFDATANONE 0   /* e_ident[EI_DATA] */
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/* elf class */
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

/* magic number */
#define EI_MAG0     0   /* e_ident[] indexes */
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6
#define EI_PAD      7

#define ELFMAG0     0x7f    /* EI_MAG */
#define ELFMAG1     'E'
#define ELFMAG2     'L'
#define ELFMAG3     'F'
#define ELFMAG      "\177ELF"

#define OLFMAG1     'O'
#define OLFMAG      "\177OLF"

typedef struct 
{
    Elf32_Word  p_type;
    Elf32_Off   p_offset;
    Elf32_Addr  p_vaddr;
    Elf32_Addr  p_paddr;
    Elf32_Word  p_filesz;
    Elf32_Word  p_memsz;
    Elf32_Word  p_flags;
    Elf32_Word  p_align;
} Elf32_Phdr;

typedef struct 
{
    Elf64_Word  p_type;
    Elf64_Word  p_flags;
    Elf64_Off   p_offset;
    Elf64_Addr  p_vaddr;
    Elf64_Addr  p_paddr;
    Elf64_Xword  p_filesz;
    Elf64_Xword  p_memsz;
    Elf64_Xword  p_align;
} Elf64_Phdr;

#define PT_NULL     0       /* p_type */
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_NUM      7


#endif
