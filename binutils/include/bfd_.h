/* Main header file for the bfd library -- portable access to object files */

#ifndef __BFD_H__
#define __BFD_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef long long bfd_int64_t;
typedef unsigned long long bfd_uint64_t;
typedef unsigned long long bfd_size_type;
typedef unsigned int flagword;  /* 32 bits of flags */
typedef unsigned char bfd_byte;
typedef unsigned long long bfd_vma;
typedef unsigned long long symvalue;
typedef long long bfd_signed_vma;
typedef unsigned long symindex;

/* An offset into a file. BFD always uses the largest possible offset
   based on the build time availability of fseek, fseeko, or fseeko64 */
typedef long long file_ptr;
typedef long long ufile_ptr;

typedef int bfd_boolean;
#undef FALSE
#undef TRUE
#define FALSE   0
#define TRUE    1

typedef struct _bfd_window_internal
{
    struct _bfd_window_internal *next;
    void *data;
    bfd_size_type size;
    int refcount : 31;      /* should be enough... */
    unsigned mapped : 1;    /* 1 = mmap, 0 = malloc */
} bfd_window_internal;

typedef struct _bfd_window
{
    /* what the user asked for */
    void *data;
    bfd_size_type size;
    /* The actual window used by BFD. Small user-requstesd read-only
       regions sharing a page may share a single window into the object
       file. Read-write versions shouldn't until I've fixed things to
       keep track of which portions have been claimed by the
       application; don't wnat to give the same region back when the
       application wants two writable copies */
    struct _bfd_window_internal *i;
} bfd_window;

/* file formats */
typedef enum bfd_format
{
    bfd_unknown = 0,    /* file format is unknown */
    bfd_object,         /* linker/assembler/compiler output */
    bfd_archive,        /* object archive file */
    bfd_core,           /* core dump */
    bfd_type_end        /* mark the end, don't use it! */
} bfd_format;

enum bfd_flavour
{
    bfd_target_unknown_flavour,
    bfd_target_aout_flavour,
    bfd_target_coff_flavour,
    bfd_target_ecoff_flavour, 
    bfd_target_xcoff_flavour,
    bfd_target_elf_flavour,
    bfd_target_ieee_flavour,
    bfd_target_nlm_flavour,
    bfd_target_oasys_flavour,
    bfd_target_tekhex_flavour,
    bfd_target_srec_flavour,
    bfd_target_ihex_flavour,
    bfd_target_som_flavour,
    bfd_target_os9k_flavour,
    bfd_target_versados_flavour,
    bfd_target_msdos_flavour,
    bfd_target_ovax_flavour,
    bfd_target_evax_flavour,
    bfd_target_mmo_flavour,
    bfd_target_mach_o_flavour,
    bfd_target_pef_flavour,
    bfd_target_pef_xlib_flavour,
    bfd_target_sym_flavour
};


/* A hash table */
struct bfd_hash_table
{
    /* the hash array */
    struct bfd_hash_entry **table;
    /* the number of slots in the hash table */
    unsigned int size;
    /* the size of elements */
    unsigned int entsize;
    /* A funcion used to create new elements in the hash table. The 
       first entry is itself a pointer to an element. When this
       function is first invoked, this pointer will be NULL. However,
       having the pointer permits a hierarchy of method functions to be
       built each of which calls the function in the superclass. Thus
       each function should be written to allocate a new block of memory
       only if the argument is NULL */
    struct bfd_hash_entry *(*newfunc)
        (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);
    /* An objalloc for this hash table. This is a struct objalloc *,
       but we use void * to avoid requiring the inclusion of objalloc.h */
    void *memory;
};


typedef struct bfd
{
    /* a unique identifier of the BFD */
    unsigned int id;

    /* the filename the application opened the BFD with */
    const char *filename;

    /* a pointer to the target jump table */
    const struct bfd_target *xvec;

    /* the IOSTREAM, and corresponding IO vector that provides access
       to the file backing the BFD */
    void *iostream;
    const struct bfd_iovec *iovec;

    /* Is the file descriptor being cached? That is, can it be closed as
       needed, and re-opened when accessed later? */
    bfd_boolean cacheable;

    /* Marks whether there was a default target specified when the
       BFD was opened. This is used to select which matching algorithm
       to use to choose the back end */
    bfd_boolean target_defaulted;

    /* The caching routines use these to maintain a
       least-recently-used list of BFDs */
    struct bfd *lru_prev, *lru_next;

    /* when a file is closed by the caching routines, BFD retains
       state information on the file here... */
    ufile_ptr where;

    /* ... and here: (``once'' means at least once) */
    bfd_boolean opened_once;

    /* set if we have a locally maintained mtime value, rather than
       getting it from the file each time */
    bfd_boolean mtime_set;

    /* file modified time, if mtime_set is TRUE */
    long mtime;

    /* reserviced for an unimplemented file locking extension */
    int ifd;

    /* the format which belongs to the BFD. (object, core, etc.) */
    bfd_format format;

    /* the direction with which the BFD was opened */
    enum bfd_direction
    {
        no_direction = 0,
        read_direction = 1,
        write_direction = 2,
        both_direction = 3
    } direction;

    /* format_specific flags */
    flagword flags;

    /* Currently my_archive is tested before adding origin to
       anything. I believe that this can become always an add of
       origin, with origin set to 0 for archive files */
    ufile_ptr origin;

    /* remember when output has begun, to stop strange things
       from happening */
    bfd_boolean output_has_begun;

    /* a hash table for section names */
    struct bfd_hash_table section_htab;

    /* pointer to linked list of sections */
    struct bfd_section *sections;

    /* the last section on the section list */
    struct bfd_section *section_last;

    /* the number of sections */
    unsigned int section_count;

    /* stuff only useful for object files:
        the start address */
    bfd_vma start_address;

    /* used for input and output */
    unsigned int symcount;

    /* symbol table for output BFD (with symcount entries) */
    struct bfd_symbol **outsymbols;

    /* used for slurped dynamic symbol tables */
    unsigned int dynsymcount;

    /* pointer to structure which contains architecture information */
    const struct bfd_arch_info *arch_info;

    /* flag set if symbols from this BFD should not be exported */
    bfd_boolean no_export;

    /* stuff only useful for archives */
    void    *arelt_data;
    struct bfd *my_archive;         /* the containing archive BFD */
    struct bfd *next;               /* the next BFD in the archive */
    struct bfd *archive_head;       /* the first BFD in the archive */
    bfd_boolean has_armap;

    /* a chain of BFD structures involved in a link */
    struct bfd *link_next;

    /* a field used by _bfd_generic_link_add_archive_symbols. This will
       be useful only for archive elements */
    int archive_pass;

    /* used by the back end to hold private data */
    union
    {
        struct aout_data_struct *aout_data;
        struct artdata *aout_ar_data;
        struct _oasys_data *oasys_obj_data;
        struct _oasys_ar_data *oasys_ar_data;
        struct coff_tdata *coff_obj_data;
        struct pe_tdata *pe_obj_data;
        struct xcoff_tdata *xcoff_obj_data;
        struct ecoff_tdata *ecoff_obj_data;
        struct ieee_data_struct *ieee_data;
        struct ieee_ar_data_struct *ieee_ar_data;
        struct srec_data_struct *srec_data;
        struct ihex_data_struct *ihex_data;
        struct tekhex_data_struct *tekhex_data;
        struct elf_obj_tdata *elf_obj_data;
        struct nlm_obj_tdata *nlm_obj_data;
        struct bout_data_struct *bout_data;
        struct mmo_data_struct *mmo_data;
        struct sun_core_struct *sun_core_data;
        struct sco5_core_struct *sco5_core_data;
        struct trad_core_struct *trad_core_data;
        struct som_data_struct *som_data;
        struct hpux_core_struct *hpux_core_data;
        struct hppabsd_core_struct *hppabsd_core_data;
        struct sgi_core_struct *sgi_core_data;
        struct lynx_core_struct *lynx_core_data;
        struct osf_core_struct *osf_core_data;
        struct cisco_core_struct *cisco_core_data;
        struct versados_data_struct *versados_data;
        struct netbsd_core_struct *netbsd_core_data;
        struct mach_o_data_struct *mach_o_data;
        struct mach_o_fat_data_struct *mach_o_fat_data;
        struct bfd_pef_data_struct *pef_data;
        struct bfd_pef_xlib_data_struct *pef_xlib_data;
        struct bfd_sym_data_struct *sym_data;
        void *any;
    } tdata;

    /* used by the application to hold private data */
    void *usrdata;

    /* Where all the allocated stuff under this BFD goes. This is a
       struct objalloc *, but we use void * to avoid requiring the inclusion
       of objalloc.h */
    void *memory;
} bfd;


/* How to handle unresolved symbols.
   There are four possibilities which are enumerated below */
enum report_method
{
    /* This is the initial value when link_info structure is created.
       It allows the various stages of the linker to determine whether they
       allowed to set the value */
    RM_NOT_YET_SET = 0,
    RM_IGNORE,
    RM_GENERATE_WARNING,
    RM_GENERATE_ERROR
};

/* which symbols to strip during a link */
enum bfd_link_strip
{
    strip_none,         /* don't strip any symbols */
    stripe_debugger,    /* strip debugging symbols */
    strip_some,         /* keep_hash is the list of symbols to keep */
    strip_all           /* strip all symbols */
};

/* Which local symbols to discard during a link. This is irrelevant
   if strip_all is used */
enum bfd_link_discard
{
    discard_sec_merge,      /* discard local temporary symbols in SEC_MERGE sections */
    discard_none,           /* don't discard any locals */
    discard_l,              /* discard local temporary symbols */
    discard_all             /* discard all locals */
};


typedef struct lineno_cache_entry
{
    unsigned int line_number;   /* Linenumber from start of function */
    union
    {
        struct bfd_symbol *sym; /* function name */
        bfd_vma offset;         /* offset into section */
    } u;
} alent;


typedef struct bfd_section
{
    /* The name of the section; the name isn't a copy, the pointer is
       the same as that passed to bfd_make_section */
    const char *name;

    /* a unique sequence number */
    int id;

    /* which section in the bfd; 0..n-1 as sections are created in a bfd */
    int index;

    /* the next section in the list belonging to the BFD, or NULL */
    struct bfd_section *next;

    /* the previous section in the list belonging to the BFD, or NULL */
    struct bfd_section *prev;

    /* The field flags contains attributes of the section. Some
       flags are read in from the object file, and some are
       synthesized from other information */
    flagword flags;

#define SEC_NO_FLAGS    0x000

    /* Tells the OS to allocate space for this section when loading.
       This is clear for a section containing debug information only */
#define SEC_ALLOC       0x001
    
    /* Tells the OS to load the section from the file when loading.
       This is clear for a .bss section */
#define SEC_LOAD        0x002

    /* The section contains data still to be relocated, so there is
       some relocation information too */
#define SEC_RELOC       0x004

    /* A signal to the OS that the section contains read only data */
#define SEC_READONLY    0x008

    /* The section contains code only */
#define SEC_CODE        0x010

    /* The section contains data only */
#define SEC_DATA        0x020

    /* The section will reside in ROM */
#define SEC_ROM         0x040

    /* The section contains constructor information. This section
       type is used by the linker to create lists of constructors and
       destructors used by <<g++>>. When a back end sees a symbol
       which should be used in a constructor list, it creates a new
       section for the type of name (e.g., <<__CTOR_LIST__>>), attaches
       the symbol to it, and builds a relocation. To build the lists
       of constructors, all the linker has to do is catenate all the
       sections called <<__CTOR_LIST__>> and relocate the data
       contained within - exactly the operations it would perform on
       standard data */
#define SEC_CONSTRUCTOR 0x080

    /* The section has contents - a data section could be
       <<SEC_ALLOC>> | <<SEC_HAS_CONTENTS>>; a debug section could be
       <<SEC_HAS_CONTENTS>> */
#define SEC_HAS_CONTENTS    0x100

    /* An instruction to the linker to not output the section
       even if it has information which would normally be written */
#define SEC_NEVER_LOAD      0x200

    /* The section contains thread local data */
#define SEC_THREAD_LOCAL    0x400

    /* The section has GOT references. This flag is only for the
       linker, and is currently only used by the elf32-hppa back end.
       It will be set if global offset table references were detected
       in this section, which indicate to the linker that the section
       contains PIC code, and must be handled specially when doing a
       static link */
#define SEC_HAS_GOT_REF     0x800

    /* The section contains common symbols (symbols may be defined
       multiple times, the value of a symbol is the amount of
       space it requires, and the largest symbol value is the one
       used). Most targets have exactly one of these (which we
       translate to bfd_com_section_ptr), but ECOFF has two. */
#define SEC_IS_COMMON       0x1000

    /* The section contains only debugging information. For
       example, this is set for ELF .debug and .stab sections.
       strip tests this flags to see if a section can be
       discarded */
#define SEC_DEBUGGING       0x2000

    /* The contents of this section are held in memory pointed to
       by the contents field. This is checked by bfd_get_section_contents,
       and the data is retrieved from memory if approriate. */
#define SEC_IN_MEMORY       0x4000

    /* The contents of this section are to be excluded by the
       linker for executable and shared objects unless those
       objects are to be further relocated */
#define SEC_EXCLUDE         0x8000

    /* The contents of this section are to be sorted based on the sum of
       the symbol and addend values specified by the associated relocation
       entries. Entries without associated relocation entries will be
       appended to the end of the section in an unspecified order */
#define SEC_SORT_ENTRIES    0x10000

    /* When linking, duplicate sections of the same name should be
       discarded, rather than being combined into a single section as
       is usually done. This is similar to how common symbols are
       handled. See SEC_LINK_DUPLICATES below */
#define SEC_LINK_ONCE       0x20000

    /* If SEC_LINK_ONCE is set, this bitfield describes how the linker
       should handle duplicate sections */
#define SEC_LINK_DUPLICATES 0x40000

    /* this value for SEC_LINK_DUPLICATES means that duplicate
       sections with the same name should simply be discarded */
#define SEC_LINK_DUPLICATES_DISCARD 0x0

    /* This value for SEC_LINK_DUPLICATES means that the linker
       should warn if there are any duplicate sections, although
       it should still only link one copy */
#define SEC_LINK_DUPLICATES_ONE_ONLY    0x80000

    /* This value for SEC_LINK_DUPLICATES means that the linker
       should warn if any duplicate sections are a different size */
#define SEC_LINK_DUPLICATES_SAME_SIZE   0x100000

    /* This value for SEC_LINK_DUPLICATES means that the linker
       should warn if any duplicate sections contain different
       contents */
#define SEC_LINK_DUPLICATES_SAME_CONTENTS   \
       (SEC_LINK_DUPLICATES_ONE_ONLY | SEC_LINK_DUPLICATES_SAME_SIZE)

    /* This section was created by the linker as part of dynamic
       relocation or other arcane processing. It is skipped when
       going through the first-pass output, trusting that someone
       else up the line will take care of it later. */
#define SEC_LINKER_CREATED  0x200000

   /* This section should not be subject to garbage collection */
#define SEC_KEEP    0x400000

   /* This section contains "short" data, and should be placed
      "near" the GP */
#define SEC_SMALL_DATA  0x800000

   /* Attempt to merge identical entities in the section.
      Entity size is given in the entsize field */
#define SEC_MERGE   0x1000000

   /* If given with SEC_MERGE, entities to merge are zero terminated
      strings where entsize specifies character size instead of fixed
      size entries */
#define SEC_STRINGS 0x2000000

   /* This section contains data about section groups */
#define SEC_GROUP   0x4000000

   /* The section is a COFF shared library section. This flag is
      only for the linker. If this type of section appears in
      the input file, the linker must copy it to the output file
      without changing the vma or size. FIXME: Althrough this
      was originally intended to be general, it really is COFF
      specific (and the flag was renamed to indicate this). It 
      might be cleaner to have some more general mechanism to
      allow the back end to control what the linker does with
      sections */
#define SEC_COFF_SHARED_LIBRARY 0x10000000
      
    /* This section contains data which may be shared with other
       executables or shared objects. This is for COFF only */
#define SEC_COFF_SHARED         0x20000000

    /* When a section with this flag is being linked, then if the size of
       the input section is less than a page, it should not cross a page
       boundary. If the size of the input section is one page or more,
       it should be aligned on a page boundary. This is for TI
       TMS320C54X only */
#define SEC_TIC54X_BLOCK        0x40000000

    /* Conditionally link this section; do not link if there are no
       references found to any symbol in the section. This is for TI
       TMS320C54X only */
#define SEC_TIC54X_CLINK        0x80000000

    /* End of section flags */

    /* Some internal packed boolean fields */

    /* See the vma field */
    unsigned int user_set_vma: 1;

    /* A mark flag used by some of the linker backends */
    unsigned int linker_mark: 1;

    /* Another mark flag used by some of the linker backends. Set for
       output sections that have an input section */
    unsigned int linker_has_input: 1;

    /* Mark flags used by some linker backends for garbage collection */
    unsigned int gc_mark: 1;
    unsigned int gc_mark_from_eh: 1;

    /* The following flags are used by the ELF linker */

    /* Mark sections which have been allocated to segments */
    unsigned int segment_mark: 1;

    /* Type of sec_info information */
    unsigned int sec_info_type: 3;
#define ELF_INFO_TYPE_NONE      0
#define ELF_INFO_TYPE_STABS     1
#define ELF_INFO_TYPE_MERGE     2
#define ELF_INFO_TYPE_EH_FRAME  3
#define ELF_INFO_TYPE_JUST_SYMS 4

    /* Nonzero if this section uses RELA relocations, rather than REL */
    unsigned int use_rela_p: 1;

    /* Bits used by various backends. The generic code doesn't touch 
       these fields */

    /* Nonzero if this section has TLS related relocations */
    unsigned int has_tls_reloc: 1;

    /* Nonzero if this section has a gp reloc */
    unsigned int has_gp_reloc: 1;

    /* Nonzero if this section needs the relax finalize pass */
    unsigned int need_finalize_relax: 1;

    /* Whether relocations have been processed */
    unsigned int reloc_done: 1;

    /* End of internal packed boolean field */

    /* The virtual memory address of the section - where it will be
       at run time. The symbols are relocated against this. The
       user_set_vma flag is maintained by bfd; if it's not set, the
       backend can assign addresses (for example, in <<a.out>>, where
       the default address for <<.data>> is dependent on the specific
       target and various flags). */
    bfd_vma vma;

    /* The load address of the section - where it would be in a
       rom image; really only used for writing section header
       information */
    bfd_vma lma;

    /* The size of the section in octets, as it will be output.
       Contains a value even if the section has no contents (e.g., the
       size of <<.bss>>). */
    bfd_size_type size;

    /* For input sections, the original size on disk of the section, in
       octets. This field is used by the linker relaxation code. It is
       currently only set for sections where the linker relaxation scheme
       doesn't cache altered section and reloc contents (stabs, eh_frame,
       SEC_MERGE, some coff relaxing targets), and thus the original size
       needs to be kept to read the section multiple times.
       For output secitons, rawsize holds the section size calculated on
       a previous linker relaxation pass */
    bfd_size_type rawsize;

    /* If this section is going to be output, then this value is the
       offset in *bytes* into the output section of the first byte in the
       input section (byte ==> smallest addressable unit on the
       target). In most cases, if this was going to start at the
       100th octet (8-bit quantity) in the output section, this value
       would be 100. However, if the target byte size is 16 bits
       (bfd_octets_per_byte is "2"), this value would be 50 */
    bfd_vma output_offset;

    /* The output section through which to map on output */
    struct bfd_section *output_section;

    /* The alignment requirement of the section, as an exponent of 2 -
       e.g., 3 aligns to 2^3 (or 8) */
    unsigned int alignment_power;

    /* If an input section, a pointer to a vector of relocation
       records for the data in this section */
    struct reloc_cache_entry *relocation;

    /* If an output seciton, a pointer to a vector of pointers to
       relocation records for the data in this section */
    struct reloc_cache_entry **orelocation;

    /* The number of relocation records in one of the above */
    unsigned reloc_count;

    /* Information below is back end specific - and not always used
       or updated */

    /* File position of section data */
    file_ptr filepos;

    /* File position of relocation info */
    file_ptr rel_filepos;

    /* File position of line data */
    file_ptr line_filepos;

    /* Pointer to data for application */
    void *userdata;

    /* If the SEC_IN_MEMORY flag if set, this points to the actual
       contents */
    unsigned char *content;

    /* Attached line number information */
    alent *lineno;

    /* Number of line number records */
    unsigned int lineno_count;

    /* Entity size of merging purposes */
    unsigned int entsize;

    /* Pointers to the kept section if this section is a link-once section,
       and is discarded */
    struct bfd_section *kept_section;

    /* When a section is being output, this value changes as more
       linenumbers are written out */
    file_ptr moving_line_filepos;

    /* What the section number is in the target world */
    int target_index;

    void *used_by_bfd;

    /* If this is a constructor section then here is a list of the
       relocations created to relocate items within it */
    struct relent_chain *constructor_chain;

    /* The BFD which owns the section */
    bfd *owner;

    /* A symbol which points at this section only */
    struct bfd_symbol *symbol;
    struct bfd_symbol **symbol_ptr_ptr;

    /* Early in the link process, map_head and map_tail are used to build
       a list of input sections attached to an output section. Later,
       output sections are these fields for a list of bfd_link_order
       struct */
    union
    {
        struct bfd_link_order *link_order;
        struct bfd_section *s;
    } map_head, map_tail;
} asection;


enum bfd_link_common_skip_ar_aymbols
{
    bfd_link_common_skip_none,
    bfd_link_common_skip_text,
    bfd_link_common_skip_data,
    bfd_link_common_skip_all
};


/* this structure holds all the information needed to communicate
   between BFD and the linker when doing a link */
typedef struct bfd_link_info
{
    /* TRUE if BFD should generate a relocatable object file */
    unsigned int relocatable: 1;

    /* TRUE if BFD should generate relocation information in the final executable */
    unsigned int emitrelocations: 1;

    /* TRUE if BFD should generate a "task linked" object file,
       similar to relocatable but also with globals converted to
       statics. */
    unsigned int task_link: 1;

    /* TRUE if BFD should generate a shared object */
    unsigned int shared: 1;

    /* TRUE if BFD should pre-bind symbols in a shared object */
    unsigned int symbolic: 1;

    /* TRUE if BFD should export all symbols in the dynamic symbol table
       of an executable, rather than only those used */
    unsigned int export_dynamic: 1;

    /* TRUE if shared objects should be linked directly, not shared */
    unsigned int static_link: 1;

    /* TRUE if the output file should be in a traditional format. This
       is equivalent to the setting of the BFD_TRADITIONAL_FORMAT flag
       on the output file, but may be checked when reading the input files */
    unsigned int traditional_format: 1;

    /* TRUE if we want to produced optimized output files. This might
       need much more time and therefore must be explicitly selected */
    unsigned int optimize: 1;

    /* TRUE if ok to have multiple definition */
    unsigned int allow_multiple_definition: 1;

    /* TRUE if ok to have version with no definition */
    unsigned int allow_undefined_version: 1;

    /* TRUE if a default symbol version should be created and used for
       exported symbols. */
    unsigned int create_default_symver: 1;

    /* TRUE if a default symbol version should be created and used for
       imported symbols */
    unsigned int default_imported_symver: 1;

    /* TRUE if symbols should be retained in memory, FALSE if they
       should be freed and reread */
    unsigned int keep_memory: 1;

    /* TRUE if every symbol should be reported back via the notice
       callback */
    unsigned int notice_all: 1;

    /* TRUE if executable should not contain copy relocs.
       Setting this true may result in a non-sharable text segment */
    unsigned int nocopyreloc: 1;

    /* TRUE if the new ELF dynamic tags are enabled */
    unsigned int new_dtages: 1;

    /* TRUE if non-PLT relocs should be merged into one reloc section
       and sorted so that relocs against the same symbol come together */
    unsigned int combreloc: 1;

    /* TRUE if .eh_frame_hdr section and PT_GNU_EH_FRAME ELF segment
       should be created */
    unsigned int eh_frame_hdr: 1;

    /* TRUE if global symbols in discarded sections should be stripped */
    unsigned int strip_descarded: 1;

    /* TRUE if generating a position independent executable */
    unsigned int pie: 1;

    /* TRUE if generating an executable, position independent or not */
    unsigned int executable: 1;

    /* TRUE if PT_GNU_STACK segment should be created with PF_R|PF_W|PF_X
       flags */
    unsigned int execstack: 1;

    /* TRUE if PT_GNU_STACK segment should be created with PF_R|PF_W flags */
    unsigned int noexecstack: 1;

    /* TRUE if PT_GNU_RELRO segment should be created */
    unsigned int relro: 1;

    /* TRUE if we should warn when adding a DT_TEXTREL to a shared object */
    unsigned int warn_shared_textrel: 1;

    /* TRUE if unreferenced sections should be removed */
    unsigned int gc_sections: 1;

    /* What to do with unresolved symbols in an object file.
       When producing executables the default is GENERATE_ERROR.
       When producing shared libraries the default is IGNORE. The
       assumption with shared libraries is that the reference will be
       resolved at load/execution time */
    enum report_method unresolved_syms_in_objects;

    /* What to do with unresolved symbols in a shared library.
       The same defaults apply */
    enum report_method unresolved_syms_in_shared_libs;

    /* which symbols to strip */
    enum bfd_link_strip strip;

    /* which local symbols to discard */
    enum bfd_link_discard discard;

    /* Criteria for skipping symbols when determining
       whether to include an object from an archive. */
    enum bfd_link_common_skip_ar_aymbols common_skip_ar_aymbols;

    /* Char that may appear as the first char of a symbol, but shoudl be
       skipped (like symbol_leading_char) when looking up symbols in
       wrap_hash. Used by PowerPC Linux for 'dot' symbols */
    char wrap_char;

    /* function callbacks */
    const struct bfd_link_callbacks *callbacks;

    /* hash table handled by BFD */
    struct bfd_link_hash_table *hash;

    /* Hash table of symbols to keep. This is NULL unless strip is
       strip_some */
    struct bfd_hash_table *keep_hash;

    /* Hash table of symbols to report back via the notice callback. If
       this is NULL, and notice_all is FALSE, then no symbols are
       reported back */
    struct bfd_hash_table *notice_hash;

    /* Hash table of symbols which are being wrapped (the --wrap linker
       option). If this is NULL, no symbols are being wrapped */
    struct bfd_hash_table *wrap_hash;

    /* The list of input BFD's involved in the link. These are chained
       together via the link_next field */
    bfd *input_bfds;

    /* If a symbol should be created for each input BFD, this is section
       where those symbols should be placed. It must be a section in
       the outuput BFD. It may be NULL, in which case no such symbols
       will be created. This is to support CREATE_OBJECT_SYMBOLS in the
       linker command language */
    asection *create_object_symbols_section;

    /* List of global symbol names that are starting points for marking
       sections against garbage collection */
    struct bfd_sym_chain *gc_sym_list;

    /* If a base output file is wanted, then this points to it */
    void *base_file;

    /* The function to call when the executable or shared object is loaded */
    const char *init_function;

    /* The function to call when the executable or shared object is unloaded */
    const char *fini_function;

    /* Number of relaxation passes. Usually only one relaxation pass
       is needed. But a backend can have as many relaxation passes as
       necessary. During bfd_relax_section call, it is set to the 
       current pass, starting from 0 */
    int relax_pass;

    /* Non-zero if auto-import thunks for DATA itmes in pei386 DLLs
       should be generated/linked against. Set to 1 if this feature
       is explicitly requested by the user, -1 if enabled by default. */
    int pei386_auto_import;

    /* Non-zero if runtime relocs for DATA items with non-zero addends
       in pei386 DLLs should be generated. Set to 1 if this feature
       is explicitly requested by the user, -1 if enabled by default */
    int pei386_runtime_pseudo_reloc;

    /* How many spare .dynamic DT_NULL entries should be added? */
    unsigned int spare_dynamic_tags;

    /* may be used to set DT_FLAGS for ELF */
    bfd_vma flags;

    /* may be used to set DT_FLAGS_1 for ELF */
    bfd_vma flags_1;

    /* start and end of RELRO region */
    bfd_vma relro_start, relro_end;
} bfd_link_info;


typedef struct bfd_symbol
{
    /* A pointer to the BFD which owns the symbol. This information
       is necessary so that  a back end can work out what additional
       information (invisible to the application writer) is carried
       with the symbol.

       This field is *almost* redundant, since you can use section->owner
       instead, except that some symbols point to the global sections
       bfd_{abs,com,und}_section. This could be fixed by making
       these globals be per-bfd (or per-target-flovor). */
    struct bfd *the_bfd;        /* use bfd_asymbol_bfd(sym) to access this field */

    /* The text of the symbol. The name is left alone, and not copied; the
       application may not alter it */
    const char *name;

    /* The value of the symbol. This really should be a union of a 
       numeric value with a pointer, since some flags indicate that
       a pointer to another symbol is stored here */
    symvalue value;

    /* Attributes of a symbol */
#define BSF_NO_FLAGS    0x00

    /* The symbol has local scope; <<static>> in <<C>>. The value
       is the offset into the section of the data */
#define BSF_LOCAL       0x01

    /* The symbol has global scope; initialized data in <<C>>. The
       value is the offset into the section of the data */
#define BSF_GLOBAL      0x02

    /* The symbol has global scope and is exported. The value is
       the offset into the section of the data */
#define BSF_EXPORT      BSF_GLOBAL  /* no real difference */

    /* A normal C symbol would be one of:
        <<BSF_LOCAL>>, <<BSF_FORMAT_COMM>>, <<BSF_UNDEFINED>> or
        <<BSF_GLOBAL>> */

    /* The symbol is a debugging record. The value has an arbitrary
       meaning, unless BSF_DEBUGGING_RELOC is also set */
#define BSF_DEBUGGING   0x08

    /* The symbol denotes a function entry point. Used in ELF,
       perhaps other someday */
#define BSF_FUNCTION    0x10

    /* used by the linker */
#define BSF_KEEP        0x20
#define BSF_KEEP_G      0x40

    /* A weak global symbol, overridable without warning by
       a regular global symbol of the same name */
#define BSF_WEAK        0x80

    /* This symbol was created to point to a section, e.g. ELF's
       STT_SECTION symbols */
#define BSF_SECTION_SYM 0x100

    /* The symbol used to be a common symbol, but now it is
       allocated */
#define BSF_OLD_COMMON  0x200

    /* The default value for common data */
#define BFD_FORT_COMM_DEFAULT_VALUE 0

    /* In some files the type of a symbol sometimes alters its
       location in an output file - ie in coff a <<ISFCN>> symbol
       which is also <<C_EXT>> symbol appears where it was
       declared and not at the end of a section. This bit is set
       by the target BFD part to convey this information */
#define BSF_NOT_AT_END  0x400

    /* Signal that the symbol is the label of constructor section */
#define BSF_CONSTRUCTOR 0x800

    /* Signal that the symbol is a warning symbol. The name is a
       warning. The name of the next symbol is the one to warn about;
       if a reference is made to a symbol with the same name as the next
       symbol, a warning is issued by the linker */
#define BSF_WARNING     0x1000

    /* Signal that the symbol is indirect. This symbol is an indirect
       pointer to the symbol with the same name as the next symbol */
#define BSF_INDIRECT    0x2000

    /* BSF_FILE marks symbols that contain a file name. This is used
       for ELF STT_FILE symbols */
#define BSF_FILE        0x4000

    /* Symbol is from dynamic linking information */
#define BSF_SYNAMIC     0x8000

    /* The symbol denotes a data object. Used in ELF, and perhaps
       others someday. */
#define BSF_OBJECT      0x10000

    /* This symbol is a debugging symbol. The value is the offset
       into the section of the data. BSF_DEBUGGING should be set
       as well */
#define BSF_DEBUGGING_RELOC 0x20000

    /* This symbol is thread local. Used in ELF */
#define BSF_THREAD_LOCAL    0x40000

    flagword flags;

    /* A pointer to the section to which this symbol is
       relative. This will always be non NULL, there are special
       sections for undefined and absolute symbols */
    struct bfd_section *section;

    /* back end special data */
    union
    {
        void *p;
        bfd_vma i;
    } udata;
} asymbol;


/* Used in generating armaps (archive tables of contents).
   Perhaps just a forward definition would do? */
struct orl      /* Output ranlib */
{
    char **name;    /* symbol name */
    union
    {
        file_ptr pos;
        bfd *abfd;
    } u;            /* bfd* or file position */
    int namidx;     /* index into string table */
};


typedef enum bfd_print_symbol
{
    bfd_print_symbol_name,
    bfd_print_symbol_more,
    bfd_print_symbol_all
} bfd_print_symbol_type;

/* information about a symbol that nm needs */
typedef struct _symbol_info
{
    symvalue value;
    char type;
    const char *name;           /* symbol name */
    unsigned char stab_type;    /* stab type */
    char stab_other;            /* stab other */
    short stab_desc;            /* stab desc */
    const char *stab_name;      /* string for stab type */
} symbol_info;

enum complain_overflow
{
    /* do not complain on overflow */
    complain_overflow_dont,

    /* Complain if the value overflows when considered as a signed
       number one bit larger than the field. ie. A bitfield of N bits
       is allowed to represent -2**n to 2**n-1 */
    complain_overflow_bitfield,

    /* Complain if the value overflows when considered as a signed
       number */
    complain_overflow_signed,

    /* Complain if the value overflows when considered as an
       unsigned number */
    complain_overflow_unsigned
};

typedef enum bfd_reloc_status
{
    /* no errors detected */
    bfd_reloc_ok,

    /* the relocation was performed, but there was an overflow */
    bfd_reloc_overflow,

    /* the address to relocate was not within the section supplied */
    bfd_reloc_outofrange,

    /* used by special function */
    bfd_reloc_continue,

    /* unsupported relocation size requested */
    bfd_reloc_notsupported,

    /* unused */
    bfd_reloc_other,

    /* the symbol to relocate against was undefined */
    bfd_reloc_undefined,

    /* The relocation was performed, but may not be ok - presently
       generated only when linking i960 coff files with i960 b.out
       symbols. If this type is returned, the error_message argument
       to bfd_perform_relocation will be set */
    bfd_reloc_dangerous
} bfd_reloc_status_type;

typedef struct reloc_cache_entry arelent;


typedef struct reloc_howto_struct
{
    /* The type field has mainly a documentary use - the back end can 
       do what it wants with it, though normally the back end's
       external idea of what a reloc number is stored
       in this field. For example, a PC relative word relocation
       in a coff environment has the type 023 - because that's
       what the outside world calls a R_PCRWORD reloc */
    unsigned int type;

    /* The value the final relocation is shifted right by. This drops
       unwanted data from the relocation */
    unsigned int rightshift;

    /* The size of the item to be relocated. This is *not* a
       power-of-two measure. To get the number of bytes operated
       on by a type of relocation, use bfd_get_reloc_size */
    int size;

    /* The number of bits in the item to be relocated. This is used
       when doing overflow checking */
    unsigned int bitsize;

    /* Note that the relocation is relative to the location in the
       data section of the addend. The relocation function will
       substract from the relocation value the address of the location
       being relocated */
    bfd_boolean pc_relative;

    /* The bit position of the reloc value in the destination.
       The relocated value is left shifted by this amount */
    unsigned int bitpos;

    /* What type of overflow error should be checked for when
       relocating */
    enum complain_overflow complain_on_overflow;

    /* If this field is non null, then the supplied function is
       called rather than the normal function. This allows really
       strange relocation methods to be accommodated (e.g., i960 callj
       instructions) */
    bfd_reloc_status_type (*special_funcion)
        (bfd *, arelent *, struct bfd_symbol *, void *, asection *,
         bfd *, char **);

    /* The textual name of the relocation type */
    char *name;

    /* Some formats record a relocation addend in the section contents
       rather than with the relocation. For ELF formats this is the
       distinction between USE_REL and USE_RELA (though the code checks
       for USE_REL == 1/0). The value of this field is TRUE if the
       addend is recorded with the section contents (the data) will be
       modified. The value of this field is FALSE if addends are
       recorded with the relocation (in arelent.addend); when performing
       a partial link the relocation will be modified.
       All relocations for all ELF USE_RELA targets should set this field
       to FALSE (values of TRUE should be looked on with suspicion).
       However, the converse is not true: not all relocations of all ELF
       USE_REL targets set this field to TRUE. Why this is so is peculiar
       to each particular target. For relocs that aren't used in partial
       links (e.g. GOT stuff) it doesn't matter what this is set to */
    bfd_boolean partial_inplace;

    /* src_mask selects the part of the instruction (or data) to be used
       in the relocation sum. If the target relocations don't have an
       addend in the reloc, eg. ELF USE_REL, src_mask will normally equal
       dst_mask to extract the addend from the section contents. If
       relocations do have an addend in the reloc, e.g. ELF USE_RELA, this
       field should be zero. Non-zero values for ELF USE_RELA targets are
       bogus as in those cases the value in the dst_mask part of the 
       section contents should be treated as garbage */
    bfd_vma src_mask;

    /* dst_mask selects which parts of the instruction (or data) are
       replaced with a relocated value */
    bfd_vma dst_mask;

    /* When some formats create PC relative instructions, they leave
       the value of the pc of the place being relocated in the offset
       slot of the instruction, so that a PC relative relocation can
       be made just  by adding in an ordinary offset (e.g., sun3 a.out).
       Some formats leave the displacement part of an instruction
       empty (e.g., m88k bcs); this flag signals the fact */
    bfd_boolean pcrel_offset;

} reloc_howto_type;


struct reloc_cache_entry
{
    /* a pointer into the canonical table of pointers */
    struct bfd_symbol **sym_ptr_ptr;

    /* offset in section */
    bfd_size_type address;

    /* addend for relocation value */
    bfd_vma addend;

    /* pointer to how to perform the required relocation */
    reloc_howto_type *howto;
};

enum bfd_reloc_code_real {
  _dummy_first_bfd_reloc_code_real,


/* Basic absolute relocations of N bits.  */
  BFD_RELOC_64,
  BFD_RELOC_32,
  BFD_RELOC_26,
  BFD_RELOC_24,
  BFD_RELOC_16,
  BFD_RELOC_14,
  BFD_RELOC_8,

/* PC-relative relocations.  Sometimes these are relative to the address
of the relocation itself; sometimes they are relative to the start of
the section containing the relocation.  It depends on the specific target.

The 24-bit relocation is used in some Intel 960 configurations.  */
  BFD_RELOC_64_PCREL,
  BFD_RELOC_32_PCREL,
  BFD_RELOC_24_PCREL,
  BFD_RELOC_16_PCREL,
  BFD_RELOC_12_PCREL,
  BFD_RELOC_8_PCREL,

/* Section relative relocations.  Some targets need this for DWARF2.  */
  BFD_RELOC_32_SECREL,

/* For ELF.  */
  BFD_RELOC_32_GOT_PCREL,
  BFD_RELOC_16_GOT_PCREL,
  BFD_RELOC_8_GOT_PCREL,
  BFD_RELOC_32_GOTOFF,
  BFD_RELOC_16_GOTOFF,
  BFD_RELOC_LO16_GOTOFF,
  BFD_RELOC_HI16_GOTOFF,
  BFD_RELOC_HI16_S_GOTOFF,
  BFD_RELOC_8_GOTOFF,
  BFD_RELOC_64_PLT_PCREL,
  BFD_RELOC_32_PLT_PCREL,
  BFD_RELOC_24_PLT_PCREL,
  BFD_RELOC_16_PLT_PCREL,
  BFD_RELOC_8_PLT_PCREL,
  BFD_RELOC_64_PLTOFF,
  BFD_RELOC_32_PLTOFF,
  BFD_RELOC_16_PLTOFF,
  BFD_RELOC_LO16_PLTOFF,
  BFD_RELOC_HI16_PLTOFF,
  BFD_RELOC_HI16_S_PLTOFF,
  BFD_RELOC_8_PLTOFF,

/* Relocations used by 68K ELF.  */
  BFD_RELOC_68K_GLOB_DAT,
  BFD_RELOC_68K_JMP_SLOT,
  BFD_RELOC_68K_RELATIVE,

/* Linkage-table relative.  */
  BFD_RELOC_32_BASEREL,
  BFD_RELOC_16_BASEREL,
  BFD_RELOC_LO16_BASEREL,
  BFD_RELOC_HI16_BASEREL,
  BFD_RELOC_HI16_S_BASEREL,
  BFD_RELOC_8_BASEREL,
  BFD_RELOC_RVA,

/* Absolute 8-bit relocation, but used to form an address like 0xFFnn.  */
  BFD_RELOC_8_FFnn,

/* These PC-relative relocations are stored as word displacements --
i.e., byte displacements shifted right two bits.  The 30-bit word
displacement (<<32_PCREL_S2>> -- 32 bits, shifted 2) is used on the
SPARC.  (SPARC tools generally refer to this as <<WDISP30>>.)  The
signed 16-bit displacement is used on the MIPS, and the 23-bit
displacement is used on the Alpha.  */
  BFD_RELOC_32_PCREL_S2,
  BFD_RELOC_16_PCREL_S2,
  BFD_RELOC_23_PCREL_S2,

/* High 22 bits and low 10 bits of 32-bit value, placed into lower bits of
the target word.  These are used on the SPARC.  */
  BFD_RELOC_HI22,
  BFD_RELOC_LO10,

/* For systems that allocate a Global Pointer register, these are
displacements off that register.  These relocation types are
handled specially, because the value the register will have is
decided relatively late.  */
  BFD_RELOC_GPREL16,
  BFD_RELOC_GPREL32,

/* Reloc types used for i960/b.out.  */
  BFD_RELOC_I960_CALLJ,

/* SPARC ELF relocations.  There is probably some overlap with other
relocation types already defined.  */
  BFD_RELOC_NONE,
  BFD_RELOC_SPARC_WDISP22,
  BFD_RELOC_SPARC22,
  BFD_RELOC_SPARC13,
  BFD_RELOC_SPARC_GOT10,
  BFD_RELOC_SPARC_GOT13,
  BFD_RELOC_SPARC_GOT22,
  BFD_RELOC_SPARC_PC10,
  BFD_RELOC_SPARC_PC22,
  BFD_RELOC_SPARC_WPLT30,
  BFD_RELOC_SPARC_COPY,
  BFD_RELOC_SPARC_GLOB_DAT,
  BFD_RELOC_SPARC_JMP_SLOT,
  BFD_RELOC_SPARC_RELATIVE,
  BFD_RELOC_SPARC_UA16,
  BFD_RELOC_SPARC_UA32,
  BFD_RELOC_SPARC_UA64,

/* I think these are specific to SPARC a.out (e.g., Sun 4).  */
  BFD_RELOC_SPARC_BASE13,
  BFD_RELOC_SPARC_BASE22,

/* SPARC64 relocations  */
#define BFD_RELOC_SPARC_64 BFD_RELOC_64
  BFD_RELOC_SPARC_10,
  BFD_RELOC_SPARC_11,
  BFD_RELOC_SPARC_OLO10,
  BFD_RELOC_SPARC_HH22,
  BFD_RELOC_SPARC_HM10,
  BFD_RELOC_SPARC_LM22,
  BFD_RELOC_SPARC_PC_HH22,
  BFD_RELOC_SPARC_PC_HM10,
  BFD_RELOC_SPARC_PC_LM22,
  BFD_RELOC_SPARC_WDISP16,
  BFD_RELOC_SPARC_WDISP19,
  BFD_RELOC_SPARC_7,
  BFD_RELOC_SPARC_6,
  BFD_RELOC_SPARC_5,
#define BFD_RELOC_SPARC_DISP64 BFD_RELOC_64_PCREL
  BFD_RELOC_SPARC_PLT32,
  BFD_RELOC_SPARC_PLT64,
  BFD_RELOC_SPARC_HIX22,
  BFD_RELOC_SPARC_LOX10,
  BFD_RELOC_SPARC_H44,
  BFD_RELOC_SPARC_M44,
  BFD_RELOC_SPARC_L44,
  BFD_RELOC_SPARC_REGISTER,

/* SPARC little endian relocation  */
  BFD_RELOC_SPARC_REV32,

/* SPARC TLS relocations  */
  BFD_RELOC_SPARC_TLS_GD_HI22,
  BFD_RELOC_SPARC_TLS_GD_LO10,
  BFD_RELOC_SPARC_TLS_GD_ADD,
  BFD_RELOC_SPARC_TLS_GD_CALL,
  BFD_RELOC_SPARC_TLS_LDM_HI22,
  BFD_RELOC_SPARC_TLS_LDM_LO10,
  BFD_RELOC_SPARC_TLS_LDM_ADD,
  BFD_RELOC_SPARC_TLS_LDM_CALL,
  BFD_RELOC_SPARC_TLS_LDO_HIX22,
  BFD_RELOC_SPARC_TLS_LDO_LOX10,
  BFD_RELOC_SPARC_TLS_LDO_ADD,
  BFD_RELOC_SPARC_TLS_IE_HI22,
  BFD_RELOC_SPARC_TLS_IE_LO10,
  BFD_RELOC_SPARC_TLS_IE_LD,
  BFD_RELOC_SPARC_TLS_IE_LDX,
  BFD_RELOC_SPARC_TLS_IE_ADD,
  BFD_RELOC_SPARC_TLS_LE_HIX22,
  BFD_RELOC_SPARC_TLS_LE_LOX10,
  BFD_RELOC_SPARC_TLS_DTPMOD32,
  BFD_RELOC_SPARC_TLS_DTPMOD64,
  BFD_RELOC_SPARC_TLS_DTPOFF32,
  BFD_RELOC_SPARC_TLS_DTPOFF64,
  BFD_RELOC_SPARC_TLS_TPOFF32,
  BFD_RELOC_SPARC_TLS_TPOFF64,

/* Alpha ECOFF and ELF relocations.  Some of these treat the symbol or
"addend" in some special way.
For GPDISP_HI16 ("gpdisp") relocations, the symbol is ignored when
writing; when reading, it will be the absolute section symbol.  The
addend is the displacement in bytes of the "lda" instruction from
the "ldah" instruction (which is at the address of this reloc).  */
  BFD_RELOC_ALPHA_GPDISP_HI16,

/* For GPDISP_LO16 ("ignore") relocations, the symbol is handled as
with GPDISP_HI16 relocs.  The addend is ignored when writing the
relocations out, and is filled in with the file's GP value on
reading, for convenience.  */
  BFD_RELOC_ALPHA_GPDISP_LO16,

/* The ELF GPDISP relocation is exactly the same as the GPDISP_HI16
relocation except that there is no accompanying GPDISP_LO16
relocation.  */
  BFD_RELOC_ALPHA_GPDISP,

/* The Alpha LITERAL/LITUSE relocs are produced by a symbol reference;
the assembler turns it into a LDQ instruction to load the address of
the symbol, and then fills in a register in the real instruction.

The LITERAL reloc, at the LDQ instruction, refers to the .lita
section symbol.  The addend is ignored when writing, but is filled
in with the file's GP value on reading, for convenience, as with the
GPDISP_LO16 reloc.

The ELF_LITERAL reloc is somewhere between 16_GOTOFF and GPDISP_LO16.
It should refer to the symbol to be referenced, as with 16_GOTOFF,
but it generates output not based on the position within the .got
section, but relative to the GP value chosen for the file during the
final link stage.

The LITUSE reloc, on the instruction using the loaded address, gives
information to the linker that it might be able to use to optimize
away some literal section references.  The symbol is ignored (read
as the absolute section symbol), and the "addend" indicates the type
of instruction using the register:
1 - "memory" fmt insn
2 - byte-manipulation (byte offset reg)
3 - jsr (target of branch)  */
  BFD_RELOC_ALPHA_LITERAL,
  BFD_RELOC_ALPHA_ELF_LITERAL,
  BFD_RELOC_ALPHA_LITUSE,

/* The HINT relocation indicates a value that should be filled into the
"hint" field of a jmp/jsr/ret instruction, for possible branch-
prediction logic which may be provided on some processors.  */
  BFD_RELOC_ALPHA_HINT,

/* The LINKAGE relocation outputs a linkage pair in the object file,
which is filled by the linker.  */
  BFD_RELOC_ALPHA_LINKAGE,

/* The CODEADDR relocation outputs a STO_CA in the object file,
which is filled by the linker.  */
  BFD_RELOC_ALPHA_CODEADDR,

/* The GPREL_HI/LO relocations together form a 32-bit offset from the
GP register.  */
  BFD_RELOC_ALPHA_GPREL_HI16,
  BFD_RELOC_ALPHA_GPREL_LO16,

/* Like BFD_RELOC_23_PCREL_S2, except that the source and target must
share a common GP, and the target address is adjusted for
STO_ALPHA_STD_GPLOAD.  */
  BFD_RELOC_ALPHA_BRSGP,

/* Alpha thread-local storage relocations.  */
  BFD_RELOC_ALPHA_TLSGD,
  BFD_RELOC_ALPHA_TLSLDM,
  BFD_RELOC_ALPHA_DTPMOD64,
  BFD_RELOC_ALPHA_GOTDTPREL16,
  BFD_RELOC_ALPHA_DTPREL64,
  BFD_RELOC_ALPHA_DTPREL_HI16,
  BFD_RELOC_ALPHA_DTPREL_LO16,
  BFD_RELOC_ALPHA_DTPREL16,
  BFD_RELOC_ALPHA_GOTTPREL16,
  BFD_RELOC_ALPHA_TPREL64,
  BFD_RELOC_ALPHA_TPREL_HI16,
  BFD_RELOC_ALPHA_TPREL_LO16,
  BFD_RELOC_ALPHA_TPREL16,

/* Bits 27..2 of the relocation address shifted right 2 bits;
simple reloc otherwise.  */
  BFD_RELOC_MIPS_JMP,

/* The MIPS16 jump instruction.  */
  BFD_RELOC_MIPS16_JMP,

/* MIPS16 GP relative reloc.  */
  BFD_RELOC_MIPS16_GPREL,

/* High 16 bits of 32-bit value; simple reloc.  */
  BFD_RELOC_HI16,

/* High 16 bits of 32-bit value but the low 16 bits will be sign
extended and added to form the final result.  If the low 16
bits form a negative number, we need to add one to the high value
to compensate for the borrow when the low bits are added.  */
  BFD_RELOC_HI16_S,

/* Low 16 bits.  */
  BFD_RELOC_LO16,

/* High 16 bits of 32-bit pc-relative value  */
  BFD_RELOC_HI16_PCREL,

/* High 16 bits of 32-bit pc-relative value, adjusted  */
  BFD_RELOC_HI16_S_PCREL,

/* Low 16 bits of pc-relative value  */
  BFD_RELOC_LO16_PCREL,

/* MIPS16 high 16 bits of 32-bit value.  */
  BFD_RELOC_MIPS16_HI16,

/* MIPS16 high 16 bits of 32-bit value but the low 16 bits will be sign
extended and added to form the final result.  If the low 16
bits form a negative number, we need to add one to the high value
to compensate for the borrow when the low bits are added.  */
  BFD_RELOC_MIPS16_HI16_S,

/* MIPS16 low 16 bits.  */
  BFD_RELOC_MIPS16_LO16,

/* Relocation against a MIPS literal section.  */
  BFD_RELOC_MIPS_LITERAL,

/* MIPS ELF relocations.  */
  BFD_RELOC_MIPS_GOT16,
  BFD_RELOC_MIPS_CALL16,
  BFD_RELOC_MIPS_GOT_HI16,
  BFD_RELOC_MIPS_GOT_LO16,
  BFD_RELOC_MIPS_CALL_HI16,
  BFD_RELOC_MIPS_CALL_LO16,
  BFD_RELOC_MIPS_SUB,
  BFD_RELOC_MIPS_GOT_PAGE,
  BFD_RELOC_MIPS_GOT_OFST,
  BFD_RELOC_MIPS_GOT_DISP,
  BFD_RELOC_MIPS_SHIFT5,
  BFD_RELOC_MIPS_SHIFT6,
  BFD_RELOC_MIPS_INSERT_A,
  BFD_RELOC_MIPS_INSERT_B,
  BFD_RELOC_MIPS_DELETE,
  BFD_RELOC_MIPS_HIGHEST,
  BFD_RELOC_MIPS_HIGHER,
  BFD_RELOC_MIPS_SCN_DISP,
  BFD_RELOC_MIPS_REL16,
  BFD_RELOC_MIPS_RELGOT,
  BFD_RELOC_MIPS_JALR,
  BFD_RELOC_MIPS_TLS_DTPMOD32,
  BFD_RELOC_MIPS_TLS_DTPREL32,
  BFD_RELOC_MIPS_TLS_DTPMOD64,
  BFD_RELOC_MIPS_TLS_DTPREL64,
  BFD_RELOC_MIPS_TLS_GD,
  BFD_RELOC_MIPS_TLS_LDM,
  BFD_RELOC_MIPS_TLS_DTPREL_HI16,
  BFD_RELOC_MIPS_TLS_DTPREL_LO16,
  BFD_RELOC_MIPS_TLS_GOTTPREL,
  BFD_RELOC_MIPS_TLS_TPREL32,
  BFD_RELOC_MIPS_TLS_TPREL64,
  BFD_RELOC_MIPS_TLS_TPREL_HI16,
  BFD_RELOC_MIPS_TLS_TPREL_LO16,


/* MIPS ELF relocations (VxWorks extensions).  */
  BFD_RELOC_MIPS_COPY,
  BFD_RELOC_MIPS_JUMP_SLOT,


/* Fujitsu Frv Relocations.  */
  BFD_RELOC_FRV_LABEL16,
  BFD_RELOC_FRV_LABEL24,
  BFD_RELOC_FRV_LO16,
  BFD_RELOC_FRV_HI16,
  BFD_RELOC_FRV_GPREL12,
  BFD_RELOC_FRV_GPRELU12,
  BFD_RELOC_FRV_GPREL32,
  BFD_RELOC_FRV_GPRELHI,
  BFD_RELOC_FRV_GPRELLO,
  BFD_RELOC_FRV_GOT12,
  BFD_RELOC_FRV_GOTHI,
  BFD_RELOC_FRV_GOTLO,
  BFD_RELOC_FRV_FUNCDESC,
  BFD_RELOC_FRV_FUNCDESC_GOT12,
  BFD_RELOC_FRV_FUNCDESC_GOTHI,
  BFD_RELOC_FRV_FUNCDESC_GOTLO,
  BFD_RELOC_FRV_FUNCDESC_VALUE,
  BFD_RELOC_FRV_FUNCDESC_GOTOFF12,
  BFD_RELOC_FRV_FUNCDESC_GOTOFFHI,
  BFD_RELOC_FRV_FUNCDESC_GOTOFFLO,
  BFD_RELOC_FRV_GOTOFF12,
  BFD_RELOC_FRV_GOTOFFHI,
  BFD_RELOC_FRV_GOTOFFLO,
  BFD_RELOC_FRV_GETTLSOFF,
  BFD_RELOC_FRV_TLSDESC_VALUE,
  BFD_RELOC_FRV_GOTTLSDESC12,
  BFD_RELOC_FRV_GOTTLSDESCHI,
  BFD_RELOC_FRV_GOTTLSDESCLO,
  BFD_RELOC_FRV_TLSMOFF12,
  BFD_RELOC_FRV_TLSMOFFHI,
  BFD_RELOC_FRV_TLSMOFFLO,
  BFD_RELOC_FRV_GOTTLSOFF12,
  BFD_RELOC_FRV_GOTTLSOFFHI,
  BFD_RELOC_FRV_GOTTLSOFFLO,
  BFD_RELOC_FRV_TLSOFF,
  BFD_RELOC_FRV_TLSDESC_RELAX,
  BFD_RELOC_FRV_GETTLSOFF_RELAX,
  BFD_RELOC_FRV_TLSOFF_RELAX,
  BFD_RELOC_FRV_TLSMOFF,


/* This is a 24bit GOT-relative reloc for the mn10300.  */
  BFD_RELOC_MN10300_GOTOFF24,

/* This is a 32bit GOT-relative reloc for the mn10300, offset by two bytes
in the instruction.  */
  BFD_RELOC_MN10300_GOT32,

/* This is a 24bit GOT-relative reloc for the mn10300, offset by two bytes
in the instruction.  */
  BFD_RELOC_MN10300_GOT24,

/* This is a 16bit GOT-relative reloc for the mn10300, offset by two bytes
in the instruction.  */
  BFD_RELOC_MN10300_GOT16,

/* Copy symbol at runtime.  */
  BFD_RELOC_MN10300_COPY,

/* Create GOT entry.  */
  BFD_RELOC_MN10300_GLOB_DAT,

/* Create PLT entry.  */
  BFD_RELOC_MN10300_JMP_SLOT,

/* Adjust by program base.  */
  BFD_RELOC_MN10300_RELATIVE,


/* i386/elf relocations  */
  BFD_RELOC_386_GOT32,
  BFD_RELOC_386_PLT32,
  BFD_RELOC_386_COPY,
  BFD_RELOC_386_GLOB_DAT,
  BFD_RELOC_386_JUMP_SLOT,
  BFD_RELOC_386_RELATIVE,
  BFD_RELOC_386_GOTOFF,
  BFD_RELOC_386_GOTPC,
  BFD_RELOC_386_TLS_TPOFF,
  BFD_RELOC_386_TLS_IE,
  BFD_RELOC_386_TLS_GOTIE,
  BFD_RELOC_386_TLS_LE,
  BFD_RELOC_386_TLS_GD,
  BFD_RELOC_386_TLS_LDM,
  BFD_RELOC_386_TLS_LDO_32,
  BFD_RELOC_386_TLS_IE_32,
  BFD_RELOC_386_TLS_LE_32,
  BFD_RELOC_386_TLS_DTPMOD32,
  BFD_RELOC_386_TLS_DTPOFF32,
  BFD_RELOC_386_TLS_TPOFF32,
  BFD_RELOC_386_TLS_GOTDESC,
  BFD_RELOC_386_TLS_DESC_CALL,
  BFD_RELOC_386_TLS_DESC,

/* x86-64/elf relocations  */
  BFD_RELOC_X86_64_GOT32,
  BFD_RELOC_X86_64_PLT32,
  BFD_RELOC_X86_64_COPY,
  BFD_RELOC_X86_64_GLOB_DAT,
  BFD_RELOC_X86_64_JUMP_SLOT,
  BFD_RELOC_X86_64_RELATIVE,
  BFD_RELOC_X86_64_GOTPCREL,
  BFD_RELOC_X86_64_32S,
  BFD_RELOC_X86_64_DTPMOD64,
  BFD_RELOC_X86_64_DTPOFF64,
  BFD_RELOC_X86_64_TPOFF64,
  BFD_RELOC_X86_64_TLSGD,
  BFD_RELOC_X86_64_TLSLD,
  BFD_RELOC_X86_64_DTPOFF32,
  BFD_RELOC_X86_64_GOTTPOFF,
  BFD_RELOC_X86_64_TPOFF32,
  BFD_RELOC_X86_64_GOTOFF64,
  BFD_RELOC_X86_64_GOTPC32,
  BFD_RELOC_X86_64_GOT64,
  BFD_RELOC_X86_64_GOTPCREL64,
  BFD_RELOC_X86_64_GOTPC64,
  BFD_RELOC_X86_64_GOTPLT64,
  BFD_RELOC_X86_64_PLTOFF64,
  BFD_RELOC_X86_64_GOTPC32_TLSDESC,
  BFD_RELOC_X86_64_TLSDESC_CALL,
  BFD_RELOC_X86_64_TLSDESC,

/* ns32k relocations  */
  BFD_RELOC_NS32K_IMM_8,
  BFD_RELOC_NS32K_IMM_16,
  BFD_RELOC_NS32K_IMM_32,
  BFD_RELOC_NS32K_IMM_8_PCREL,
  BFD_RELOC_NS32K_IMM_16_PCREL,
  BFD_RELOC_NS32K_IMM_32_PCREL,
  BFD_RELOC_NS32K_DISP_8,
  BFD_RELOC_NS32K_DISP_16,
  BFD_RELOC_NS32K_DISP_32,
  BFD_RELOC_NS32K_DISP_8_PCREL,
  BFD_RELOC_NS32K_DISP_16_PCREL,
  BFD_RELOC_NS32K_DISP_32_PCREL,

/* PDP11 relocations  */
  BFD_RELOC_PDP11_DISP_8_PCREL,
  BFD_RELOC_PDP11_DISP_6_PCREL,

/* Picojava relocs.  Not all of these appear in object files.  */
  BFD_RELOC_PJ_CODE_HI16,
  BFD_RELOC_PJ_CODE_LO16,
  BFD_RELOC_PJ_CODE_DIR16,
  BFD_RELOC_PJ_CODE_DIR32,
  BFD_RELOC_PJ_CODE_REL16,
  BFD_RELOC_PJ_CODE_REL32,

/* Power(rs6000) and PowerPC relocations.  */
  BFD_RELOC_PPC_B26,
  BFD_RELOC_PPC_BA26,
  BFD_RELOC_PPC_TOC16,
  BFD_RELOC_PPC_B16,
  BFD_RELOC_PPC_B16_BRTAKEN,
  BFD_RELOC_PPC_B16_BRNTAKEN,
  BFD_RELOC_PPC_BA16,
  BFD_RELOC_PPC_BA16_BRTAKEN,
  BFD_RELOC_PPC_BA16_BRNTAKEN,
  BFD_RELOC_PPC_COPY,
  BFD_RELOC_PPC_GLOB_DAT,
  BFD_RELOC_PPC_JMP_SLOT,
  BFD_RELOC_PPC_RELATIVE,
  BFD_RELOC_PPC_LOCAL24PC,
  BFD_RELOC_PPC_EMB_NADDR32,
  BFD_RELOC_PPC_EMB_NADDR16,
  BFD_RELOC_PPC_EMB_NADDR16_LO,
  BFD_RELOC_PPC_EMB_NADDR16_HI,
  BFD_RELOC_PPC_EMB_NADDR16_HA,
  BFD_RELOC_PPC_EMB_SDAI16,
  BFD_RELOC_PPC_EMB_SDA2I16,
  BFD_RELOC_PPC_EMB_SDA2REL,
  BFD_RELOC_PPC_EMB_SDA21,
  BFD_RELOC_PPC_EMB_MRKREF,
  BFD_RELOC_PPC_EMB_RELSEC16,
  BFD_RELOC_PPC_EMB_RELST_LO,
  BFD_RELOC_PPC_EMB_RELST_HI,
  BFD_RELOC_PPC_EMB_RELST_HA,
  BFD_RELOC_PPC_EMB_BIT_FLD,
  BFD_RELOC_PPC_EMB_RELSDA,
  BFD_RELOC_PPC64_HIGHER,
  BFD_RELOC_PPC64_HIGHER_S,
  BFD_RELOC_PPC64_HIGHEST,
  BFD_RELOC_PPC64_HIGHEST_S,
  BFD_RELOC_PPC64_TOC16_LO,
  BFD_RELOC_PPC64_TOC16_HI,
  BFD_RELOC_PPC64_TOC16_HA,
  BFD_RELOC_PPC64_TOC,
  BFD_RELOC_PPC64_PLTGOT16,
  BFD_RELOC_PPC64_PLTGOT16_LO,
  BFD_RELOC_PPC64_PLTGOT16_HI,
  BFD_RELOC_PPC64_PLTGOT16_HA,
  BFD_RELOC_PPC64_ADDR16_DS,
  BFD_RELOC_PPC64_ADDR16_LO_DS,
  BFD_RELOC_PPC64_GOT16_DS,
  BFD_RELOC_PPC64_GOT16_LO_DS,
  BFD_RELOC_PPC64_PLT16_LO_DS,
  BFD_RELOC_PPC64_SECTOFF_DS,
  BFD_RELOC_PPC64_SECTOFF_LO_DS,
  BFD_RELOC_PPC64_TOC16_DS,
  BFD_RELOC_PPC64_TOC16_LO_DS,
  BFD_RELOC_PPC64_PLTGOT16_DS,
  BFD_RELOC_PPC64_PLTGOT16_LO_DS,

/* PowerPC and PowerPC64 thread-local storage relocations.  */
  BFD_RELOC_PPC_TLS,
  BFD_RELOC_PPC_DTPMOD,
  BFD_RELOC_PPC_TPREL16,
  BFD_RELOC_PPC_TPREL16_LO,
  BFD_RELOC_PPC_TPREL16_HI,
  BFD_RELOC_PPC_TPREL16_HA,
  BFD_RELOC_PPC_TPREL,
  BFD_RELOC_PPC_DTPREL16,
  BFD_RELOC_PPC_DTPREL16_LO,
  BFD_RELOC_PPC_DTPREL16_HI,
  BFD_RELOC_PPC_DTPREL16_HA,
  BFD_RELOC_PPC_DTPREL,
  BFD_RELOC_PPC_GOT_TLSGD16,
  BFD_RELOC_PPC_GOT_TLSGD16_LO,
  BFD_RELOC_PPC_GOT_TLSGD16_HI,
  BFD_RELOC_PPC_GOT_TLSGD16_HA,
  BFD_RELOC_PPC_GOT_TLSLD16,
  BFD_RELOC_PPC_GOT_TLSLD16_LO,
  BFD_RELOC_PPC_GOT_TLSLD16_HI,
  BFD_RELOC_PPC_GOT_TLSLD16_HA,
  BFD_RELOC_PPC_GOT_TPREL16,
  BFD_RELOC_PPC_GOT_TPREL16_LO,
  BFD_RELOC_PPC_GOT_TPREL16_HI,
  BFD_RELOC_PPC_GOT_TPREL16_HA,
  BFD_RELOC_PPC_GOT_DTPREL16,
  BFD_RELOC_PPC_GOT_DTPREL16_LO,
  BFD_RELOC_PPC_GOT_DTPREL16_HI,
  BFD_RELOC_PPC_GOT_DTPREL16_HA,
  BFD_RELOC_PPC64_TPREL16_DS,
  BFD_RELOC_PPC64_TPREL16_LO_DS,
  BFD_RELOC_PPC64_TPREL16_HIGHER,
  BFD_RELOC_PPC64_TPREL16_HIGHERA,
  BFD_RELOC_PPC64_TPREL16_HIGHEST,
  BFD_RELOC_PPC64_TPREL16_HIGHESTA,
  BFD_RELOC_PPC64_DTPREL16_DS,
  BFD_RELOC_PPC64_DTPREL16_LO_DS,
  BFD_RELOC_PPC64_DTPREL16_HIGHER,
  BFD_RELOC_PPC64_DTPREL16_HIGHERA,
  BFD_RELOC_PPC64_DTPREL16_HIGHEST,
  BFD_RELOC_PPC64_DTPREL16_HIGHESTA,

/* IBM 370/390 relocations  */
  BFD_RELOC_I370_D12,

/* The type of reloc used to build a constructor table - at the moment
probably a 32 bit wide absolute relocation, but the target can choose.
It generally does map to one of the other relocation types.  */
  BFD_RELOC_CTOR,

/* ARM 26 bit pc-relative branch.  The lowest two bits must be zero and are
not stored in the instruction.  */
  BFD_RELOC_ARM_PCREL_BRANCH,

/* ARM 26 bit pc-relative branch.  The lowest bit must be zero and is
not stored in the instruction.  The 2nd lowest bit comes from a 1 bit
field in the instruction.  */
  BFD_RELOC_ARM_PCREL_BLX,

/* Thumb 22 bit pc-relative branch.  The lowest bit must be zero and is
not stored in the instruction.  The 2nd lowest bit comes from a 1 bit
field in the instruction.  */
  BFD_RELOC_THUMB_PCREL_BLX,

/* ARM 26-bit pc-relative branch for an unconditional BL or BLX instruction.  */
  BFD_RELOC_ARM_PCREL_CALL,

/* ARM 26-bit pc-relative branch for B or conditional BL instruction.  */
  BFD_RELOC_ARM_PCREL_JUMP,

/* Thumb 7-, 9-, 12-, 20-, 23-, and 25-bit pc-relative branches.
The lowest bit must be zero and is not stored in the instruction.
Note that the corresponding ELF R_ARM_THM_JUMPnn constant has an
"nn" one smaller in all cases.  Note further that BRANCH23
corresponds to R_ARM_THM_CALL.  */
  BFD_RELOC_THUMB_PCREL_BRANCH7,
  BFD_RELOC_THUMB_PCREL_BRANCH9,
  BFD_RELOC_THUMB_PCREL_BRANCH12,
  BFD_RELOC_THUMB_PCREL_BRANCH20,
  BFD_RELOC_THUMB_PCREL_BRANCH23,
  BFD_RELOC_THUMB_PCREL_BRANCH25,

/* 12-bit immediate offset, used in ARM-format ldr and str instructions.  */
  BFD_RELOC_ARM_OFFSET_IMM,

/* 5-bit immediate offset, used in Thumb-format ldr and str instructions.  */
  BFD_RELOC_ARM_THUMB_OFFSET,

/* Pc-relative or absolute relocation depending on target.  Used for
entries in .init_array sections.  */
  BFD_RELOC_ARM_TARGET1,

/* Read-only segment base relative address.  */
  BFD_RELOC_ARM_ROSEGREL32,

/* Data segment base relative address.  */
  BFD_RELOC_ARM_SBREL32,

/* This reloc is used for references to RTTI data from exception handling
tables.  The actual definition depends on the target.  It may be a
pc-relative or some form of GOT-indirect relocation.  */
  BFD_RELOC_ARM_TARGET2,

/* 31-bit PC relative address.  */
  BFD_RELOC_ARM_PREL31,

/* Relocations for setting up GOTs and PLTs for shared libraries.  */
  BFD_RELOC_ARM_JUMP_SLOT,
  BFD_RELOC_ARM_GLOB_DAT,
  BFD_RELOC_ARM_GOT32,
  BFD_RELOC_ARM_PLT32,
  BFD_RELOC_ARM_RELATIVE,
  BFD_RELOC_ARM_GOTOFF,
  BFD_RELOC_ARM_GOTPC,

/* ARM thread-local storage relocations.  */
  BFD_RELOC_ARM_TLS_GD32,
  BFD_RELOC_ARM_TLS_LDO32,
  BFD_RELOC_ARM_TLS_LDM32,
  BFD_RELOC_ARM_TLS_DTPOFF32,
  BFD_RELOC_ARM_TLS_DTPMOD32,
  BFD_RELOC_ARM_TLS_TPOFF32,
  BFD_RELOC_ARM_TLS_IE32,
  BFD_RELOC_ARM_TLS_LE32,

/* These relocs are only used within the ARM assembler.  They are not
(at present) written to any object files.  */
  BFD_RELOC_ARM_IMMEDIATE,
  BFD_RELOC_ARM_ADRL_IMMEDIATE,
  BFD_RELOC_ARM_T32_IMMEDIATE,
  BFD_RELOC_ARM_T32_IMM12,
  BFD_RELOC_ARM_T32_ADD_PC12,
  BFD_RELOC_ARM_SHIFT_IMM,
  BFD_RELOC_ARM_SMC,
  BFD_RELOC_ARM_SWI,
  BFD_RELOC_ARM_MULTI,
  BFD_RELOC_ARM_CP_OFF_IMM,
  BFD_RELOC_ARM_CP_OFF_IMM_S2,
  BFD_RELOC_ARM_T32_CP_OFF_IMM,
  BFD_RELOC_ARM_T32_CP_OFF_IMM_S2,
  BFD_RELOC_ARM_ADR_IMM,
  BFD_RELOC_ARM_LDR_IMM,
  BFD_RELOC_ARM_LITERAL,
  BFD_RELOC_ARM_IN_POOL,
  BFD_RELOC_ARM_OFFSET_IMM8,
  BFD_RELOC_ARM_T32_OFFSET_U8,
  BFD_RELOC_ARM_T32_OFFSET_IMM,
  BFD_RELOC_ARM_HWLITERAL,
  BFD_RELOC_ARM_THUMB_ADD,
  BFD_RELOC_ARM_THUMB_IMM,
  BFD_RELOC_ARM_THUMB_SHIFT,

/* Renesas / SuperH SH relocs.  Not all of these appear in object files.  */
  BFD_RELOC_SH_PCDISP8BY2,
  BFD_RELOC_SH_PCDISP12BY2,
  BFD_RELOC_SH_IMM3,
  BFD_RELOC_SH_IMM3U,
  BFD_RELOC_SH_DISP12,
  BFD_RELOC_SH_DISP12BY2,
  BFD_RELOC_SH_DISP12BY4,
  BFD_RELOC_SH_DISP12BY8,
  BFD_RELOC_SH_DISP20,
  BFD_RELOC_SH_DISP20BY8,
  BFD_RELOC_SH_IMM4,
  BFD_RELOC_SH_IMM4BY2,
  BFD_RELOC_SH_IMM4BY4,
  BFD_RELOC_SH_IMM8,
  BFD_RELOC_SH_IMM8BY2,
  BFD_RELOC_SH_IMM8BY4,
  BFD_RELOC_SH_PCRELIMM8BY2,
  BFD_RELOC_SH_PCRELIMM8BY4,
  BFD_RELOC_SH_SWITCH16,
  BFD_RELOC_SH_SWITCH32,
  BFD_RELOC_SH_USES,
  BFD_RELOC_SH_COUNT,
  BFD_RELOC_SH_ALIGN,
  BFD_RELOC_SH_CODE,
  BFD_RELOC_SH_DATA,
  BFD_RELOC_SH_LABEL,
  BFD_RELOC_SH_LOOP_START,
  BFD_RELOC_SH_LOOP_END,
  BFD_RELOC_SH_COPY,
  BFD_RELOC_SH_GLOB_DAT,
  BFD_RELOC_SH_JMP_SLOT,
  BFD_RELOC_SH_RELATIVE,
  BFD_RELOC_SH_GOTPC,
  BFD_RELOC_SH_GOT_LOW16,
  BFD_RELOC_SH_GOT_MEDLOW16,
  BFD_RELOC_SH_GOT_MEDHI16,
  BFD_RELOC_SH_GOT_HI16,
  BFD_RELOC_SH_GOTPLT_LOW16,
  BFD_RELOC_SH_GOTPLT_MEDLOW16,
  BFD_RELOC_SH_GOTPLT_MEDHI16,
  BFD_RELOC_SH_GOTPLT_HI16,
  BFD_RELOC_SH_PLT_LOW16,
  BFD_RELOC_SH_PLT_MEDLOW16,
  BFD_RELOC_SH_PLT_MEDHI16,
  BFD_RELOC_SH_PLT_HI16,
  BFD_RELOC_SH_GOTOFF_LOW16,
  BFD_RELOC_SH_GOTOFF_MEDLOW16,
  BFD_RELOC_SH_GOTOFF_MEDHI16,
  BFD_RELOC_SH_GOTOFF_HI16,
  BFD_RELOC_SH_GOTPC_LOW16,
  BFD_RELOC_SH_GOTPC_MEDLOW16,
  BFD_RELOC_SH_GOTPC_MEDHI16,
  BFD_RELOC_SH_GOTPC_HI16,
  BFD_RELOC_SH_COPY64,
  BFD_RELOC_SH_GLOB_DAT64,
  BFD_RELOC_SH_JMP_SLOT64,
  BFD_RELOC_SH_RELATIVE64,
  BFD_RELOC_SH_GOT10BY4,
  BFD_RELOC_SH_GOT10BY8,
  BFD_RELOC_SH_GOTPLT10BY4,
  BFD_RELOC_SH_GOTPLT10BY8,
  BFD_RELOC_SH_GOTPLT32,
  BFD_RELOC_SH_SHMEDIA_CODE,
  BFD_RELOC_SH_IMMU5,
  BFD_RELOC_SH_IMMS6,
  BFD_RELOC_SH_IMMS6BY32,
  BFD_RELOC_SH_IMMU6,
  BFD_RELOC_SH_IMMS10,
  BFD_RELOC_SH_IMMS10BY2,
  BFD_RELOC_SH_IMMS10BY4,
  BFD_RELOC_SH_IMMS10BY8,
  BFD_RELOC_SH_IMMS16,
  BFD_RELOC_SH_IMMU16,
  BFD_RELOC_SH_IMM_LOW16,
  BFD_RELOC_SH_IMM_LOW16_PCREL,
  BFD_RELOC_SH_IMM_MEDLOW16,
  BFD_RELOC_SH_IMM_MEDLOW16_PCREL,
  BFD_RELOC_SH_IMM_MEDHI16,
  BFD_RELOC_SH_IMM_MEDHI16_PCREL,
  BFD_RELOC_SH_IMM_HI16,
  BFD_RELOC_SH_IMM_HI16_PCREL,
  BFD_RELOC_SH_PT_16,
  BFD_RELOC_SH_TLS_GD_32,
  BFD_RELOC_SH_TLS_LD_32,
  BFD_RELOC_SH_TLS_LDO_32,
  BFD_RELOC_SH_TLS_IE_32,
  BFD_RELOC_SH_TLS_LE_32,
  BFD_RELOC_SH_TLS_DTPMOD32,
  BFD_RELOC_SH_TLS_DTPOFF32,
  BFD_RELOC_SH_TLS_TPOFF32,

/* ARC Cores relocs.
ARC 22 bit pc-relative branch.  The lowest two bits must be zero and are
not stored in the instruction.  The high 20 bits are installed in bits 26
through 7 of the instruction.  */
  BFD_RELOC_ARC_B22_PCREL,

/* ARC 26 bit absolute branch.  The lowest two bits must be zero and are not
stored in the instruction.  The high 24 bits are installed in bits 23
through 0.  */
  BFD_RELOC_ARC_B26,

/* ADI Blackfin 16 bit immediate absolute reloc.  */
  BFD_RELOC_BFIN_16_IMM,

/* ADI Blackfin 16 bit immediate absolute reloc higher 16 bits.  */
  BFD_RELOC_BFIN_16_HIGH,

/* ADI Blackfin 'a' part of LSETUP.  */
  BFD_RELOC_BFIN_4_PCREL,

/* ADI Blackfin.  */
  BFD_RELOC_BFIN_5_PCREL,

/* ADI Blackfin 16 bit immediate absolute reloc lower 16 bits.  */
  BFD_RELOC_BFIN_16_LOW,

/* ADI Blackfin.  */
  BFD_RELOC_BFIN_10_PCREL,

/* ADI Blackfin 'b' part of LSETUP.  */
  BFD_RELOC_BFIN_11_PCREL,

/* ADI Blackfin.  */
  BFD_RELOC_BFIN_12_PCREL_JUMP,

/* ADI Blackfin Short jump, pcrel.  */
  BFD_RELOC_BFIN_12_PCREL_JUMP_S,

/* ADI Blackfin Call.x not implemented.  */
  BFD_RELOC_BFIN_24_PCREL_CALL_X,

/* ADI Blackfin Long Jump pcrel.  */
  BFD_RELOC_BFIN_24_PCREL_JUMP_L,

/* ADI Blackfin FD-PIC relocations.  */
  BFD_RELOC_BFIN_GOT17M4,
  BFD_RELOC_BFIN_GOTHI,
  BFD_RELOC_BFIN_GOTLO,
  BFD_RELOC_BFIN_FUNCDESC,
  BFD_RELOC_BFIN_FUNCDESC_GOT17M4,
  BFD_RELOC_BFIN_FUNCDESC_GOTHI,
  BFD_RELOC_BFIN_FUNCDESC_GOTLO,
  BFD_RELOC_BFIN_FUNCDESC_VALUE,
  BFD_RELOC_BFIN_FUNCDESC_GOTOFF17M4,
  BFD_RELOC_BFIN_FUNCDESC_GOTOFFHI,
  BFD_RELOC_BFIN_FUNCDESC_GOTOFFLO,
  BFD_RELOC_BFIN_GOTOFF17M4,
  BFD_RELOC_BFIN_GOTOFFHI,
  BFD_RELOC_BFIN_GOTOFFLO,

/* ADI Blackfin GOT relocation.  */
  BFD_RELOC_BFIN_GOT,

/* ADI Blackfin PLTPC relocation.  */
  BFD_RELOC_BFIN_PLTPC,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_PUSH,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_CONST,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_ADD,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_SUB,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_MULT,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_DIV,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_MOD,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_LSHIFT,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_RSHIFT,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_AND,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_OR,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_XOR,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_LAND,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_LOR,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_LEN,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_NEG,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_COMP,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_PAGE,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_HWPAGE,

/* ADI Blackfin arithmetic relocation.  */
  BFD_ARELOC_BFIN_ADDR,

/* Mitsubishi D10V relocs.
This is a 10-bit reloc with the right 2 bits
assumed to be 0.  */
  BFD_RELOC_D10V_10_PCREL_R,

/* Mitsubishi D10V relocs.
This is a 10-bit reloc with the right 2 bits
assumed to be 0.  This is the same as the previous reloc
except it is in the left container, i.e.,
shifted left 15 bits.  */
  BFD_RELOC_D10V_10_PCREL_L,

/* This is an 18-bit reloc with the right 2 bits
assumed to be 0.  */
  BFD_RELOC_D10V_18,

/* This is an 18-bit reloc with the right 2 bits
assumed to be 0.  */
  BFD_RELOC_D10V_18_PCREL,

/* Mitsubishi D30V relocs.
This is a 6-bit absolute reloc.  */
  BFD_RELOC_D30V_6,

/* This is a 6-bit pc-relative reloc with
the right 3 bits assumed to be 0.  */
  BFD_RELOC_D30V_9_PCREL,

/* This is a 6-bit pc-relative reloc with
the right 3 bits assumed to be 0. Same
as the previous reloc but on the right side
of the container.  */
  BFD_RELOC_D30V_9_PCREL_R,

/* This is a 12-bit absolute reloc with the
right 3 bitsassumed to be 0.  */
  BFD_RELOC_D30V_15,

/* This is a 12-bit pc-relative reloc with
the right 3 bits assumed to be 0.  */
  BFD_RELOC_D30V_15_PCREL,

/* This is a 12-bit pc-relative reloc with
the right 3 bits assumed to be 0. Same
as the previous reloc but on the right side
of the container.  */
  BFD_RELOC_D30V_15_PCREL_R,

/* This is an 18-bit absolute reloc with
the right 3 bits assumed to be 0.  */
  BFD_RELOC_D30V_21,

/* This is an 18-bit pc-relative reloc with
the right 3 bits assumed to be 0.  */
  BFD_RELOC_D30V_21_PCREL,

/* This is an 18-bit pc-relative reloc with
the right 3 bits assumed to be 0. Same
as the previous reloc but on the right side
of the container.  */
  BFD_RELOC_D30V_21_PCREL_R,

/* This is a 32-bit absolute reloc.  */
  BFD_RELOC_D30V_32,

/* This is a 32-bit pc-relative reloc.  */
  BFD_RELOC_D30V_32_PCREL,

/* DLX relocs  */
  BFD_RELOC_DLX_HI16_S,

/* DLX relocs  */
  BFD_RELOC_DLX_LO16,

/* DLX relocs  */
  BFD_RELOC_DLX_JMP26,

/* Renesas M16C/M32C Relocations.  */
  BFD_RELOC_M32C_HI8,
  BFD_RELOC_M32C_RL_JUMP,
  BFD_RELOC_M32C_RL_1ADDR,
  BFD_RELOC_M32C_RL_2ADDR,

/* Renesas M32R (formerly Mitsubishi M32R) relocs.
This is a 24 bit absolute address.  */
  BFD_RELOC_M32R_24,

/* This is a 10-bit pc-relative reloc with the right 2 bits assumed to be 0.  */
  BFD_RELOC_M32R_10_PCREL,

/* This is an 18-bit reloc with the right 2 bits assumed to be 0.  */
  BFD_RELOC_M32R_18_PCREL,

/* This is a 26-bit reloc with the right 2 bits assumed to be 0.  */
  BFD_RELOC_M32R_26_PCREL,

/* This is a 16-bit reloc containing the high 16 bits of an address
used when the lower 16 bits are treated as unsigned.  */
  BFD_RELOC_M32R_HI16_ULO,

/* This is a 16-bit reloc containing the high 16 bits of an address
used when the lower 16 bits are treated as signed.  */
  BFD_RELOC_M32R_HI16_SLO,

/* This is a 16-bit reloc containing the lower 16 bits of an address.  */
  BFD_RELOC_M32R_LO16,

/* This is a 16-bit reloc containing the small data area offset for use in
add3, load, and store instructions.  */
  BFD_RELOC_M32R_SDA16,

/* For PIC.  */
  BFD_RELOC_M32R_GOT24,
  BFD_RELOC_M32R_26_PLTREL,
  BFD_RELOC_M32R_COPY,
  BFD_RELOC_M32R_GLOB_DAT,
  BFD_RELOC_M32R_JMP_SLOT,
  BFD_RELOC_M32R_RELATIVE,
  BFD_RELOC_M32R_GOTOFF,
  BFD_RELOC_M32R_GOTOFF_HI_ULO,
  BFD_RELOC_M32R_GOTOFF_HI_SLO,
  BFD_RELOC_M32R_GOTOFF_LO,
  BFD_RELOC_M32R_GOTPC24,
  BFD_RELOC_M32R_GOT16_HI_ULO,
  BFD_RELOC_M32R_GOT16_HI_SLO,
  BFD_RELOC_M32R_GOT16_LO,
  BFD_RELOC_M32R_GOTPC_HI_ULO,
  BFD_RELOC_M32R_GOTPC_HI_SLO,
  BFD_RELOC_M32R_GOTPC_LO,

/* This is a 9-bit reloc  */
  BFD_RELOC_V850_9_PCREL,

/* This is a 22-bit reloc  */
  BFD_RELOC_V850_22_PCREL,

/* This is a 16 bit offset from the short data area pointer.  */
  BFD_RELOC_V850_SDA_16_16_OFFSET,

/* This is a 16 bit offset (of which only 15 bits are used) from the
short data area pointer.  */
  BFD_RELOC_V850_SDA_15_16_OFFSET,

/* This is a 16 bit offset from the zero data area pointer.  */
  BFD_RELOC_V850_ZDA_16_16_OFFSET,

/* This is a 16 bit offset (of which only 15 bits are used) from the
zero data area pointer.  */
  BFD_RELOC_V850_ZDA_15_16_OFFSET,

/* This is an 8 bit offset (of which only 6 bits are used) from the
tiny data area pointer.  */
  BFD_RELOC_V850_TDA_6_8_OFFSET,

/* This is an 8bit offset (of which only 7 bits are used) from the tiny
data area pointer.  */
  BFD_RELOC_V850_TDA_7_8_OFFSET,

/* This is a 7 bit offset from the tiny data area pointer.  */
  BFD_RELOC_V850_TDA_7_7_OFFSET,

/* This is a 16 bit offset from the tiny data area pointer.  */
  BFD_RELOC_V850_TDA_16_16_OFFSET,

/* This is a 5 bit offset (of which only 4 bits are used) from the tiny
data area pointer.  */
  BFD_RELOC_V850_TDA_4_5_OFFSET,

/* This is a 4 bit offset from the tiny data area pointer.  */
  BFD_RELOC_V850_TDA_4_4_OFFSET,

/* This is a 16 bit offset from the short data area pointer, with the
bits placed non-contiguously in the instruction.  */
  BFD_RELOC_V850_SDA_16_16_SPLIT_OFFSET,

/* This is a 16 bit offset from the zero data area pointer, with the
bits placed non-contiguously in the instruction.  */
  BFD_RELOC_V850_ZDA_16_16_SPLIT_OFFSET,

/* This is a 6 bit offset from the call table base pointer.  */
  BFD_RELOC_V850_CALLT_6_7_OFFSET,

/* This is a 16 bit offset from the call table base pointer.  */
  BFD_RELOC_V850_CALLT_16_16_OFFSET,

/* Used for relaxing indirect function calls.  */
  BFD_RELOC_V850_LONGCALL,

/* Used for relaxing indirect jumps.  */
  BFD_RELOC_V850_LONGJUMP,

/* Used to maintain alignment whilst relaxing.  */
  BFD_RELOC_V850_ALIGN,

/* This is a variation of BFD_RELOC_LO16 that can be used in v850e ld.bu
instructions.  */
  BFD_RELOC_V850_LO16_SPLIT_OFFSET,

/* This is a 32bit pcrel reloc for the mn10300, offset by two bytes in the
instruction.  */
  BFD_RELOC_MN10300_32_PCREL,

/* This is a 16bit pcrel reloc for the mn10300, offset by two bytes in the
instruction.  */
  BFD_RELOC_MN10300_16_PCREL,

/* This is a 8bit DP reloc for the tms320c30, where the most
significant 8 bits of a 24 bit word are placed into the least
significant 8 bits of the opcode.  */
  BFD_RELOC_TIC30_LDP,

/* This is a 7bit reloc for the tms320c54x, where the least
significant 7 bits of a 16 bit word are placed into the least
significant 7 bits of the opcode.  */
  BFD_RELOC_TIC54X_PARTLS7,

/* This is a 9bit DP reloc for the tms320c54x, where the most
significant 9 bits of a 16 bit word are placed into the least
significant 9 bits of the opcode.  */
  BFD_RELOC_TIC54X_PARTMS9,

/* This is an extended address 23-bit reloc for the tms320c54x.  */
  BFD_RELOC_TIC54X_23,

/* This is a 16-bit reloc for the tms320c54x, where the least
significant 16 bits of a 23-bit extended address are placed into
the opcode.  */
  BFD_RELOC_TIC54X_16_OF_23,

/* This is a reloc for the tms320c54x, where the most
significant 7 bits of a 23-bit extended address are placed into
the opcode.  */
  BFD_RELOC_TIC54X_MS7_OF_23,

/* This is a 48 bit reloc for the FR30 that stores 32 bits.  */
  BFD_RELOC_FR30_48,

/* This is a 32 bit reloc for the FR30 that stores 20 bits split up into
two sections.  */
  BFD_RELOC_FR30_20,

/* This is a 16 bit reloc for the FR30 that stores a 6 bit word offset in
4 bits.  */
  BFD_RELOC_FR30_6_IN_4,

/* This is a 16 bit reloc for the FR30 that stores an 8 bit byte offset
into 8 bits.  */
  BFD_RELOC_FR30_8_IN_8,

/* This is a 16 bit reloc for the FR30 that stores a 9 bit short offset
into 8 bits.  */
  BFD_RELOC_FR30_9_IN_8,

/* This is a 16 bit reloc for the FR30 that stores a 10 bit word offset
into 8 bits.  */
  BFD_RELOC_FR30_10_IN_8,

/* This is a 16 bit reloc for the FR30 that stores a 9 bit pc relative
short offset into 8 bits.  */
  BFD_RELOC_FR30_9_PCREL,

/* This is a 16 bit reloc for the FR30 that stores a 12 bit pc relative
short offset into 11 bits.  */
  BFD_RELOC_FR30_12_PCREL,

/* Motorola Mcore relocations.  */
  BFD_RELOC_MCORE_PCREL_IMM8BY4,
  BFD_RELOC_MCORE_PCREL_IMM11BY2,
  BFD_RELOC_MCORE_PCREL_IMM4BY2,
  BFD_RELOC_MCORE_PCREL_32,
  BFD_RELOC_MCORE_PCREL_JSR_IMM11BY2,
  BFD_RELOC_MCORE_RVA,

/* These are relocations for the GETA instruction.  */
  BFD_RELOC_MMIX_GETA,
  BFD_RELOC_MMIX_GETA_1,
  BFD_RELOC_MMIX_GETA_2,
  BFD_RELOC_MMIX_GETA_3,

/* These are relocations for a conditional branch instruction.  */
  BFD_RELOC_MMIX_CBRANCH,
  BFD_RELOC_MMIX_CBRANCH_J,
  BFD_RELOC_MMIX_CBRANCH_1,
  BFD_RELOC_MMIX_CBRANCH_2,
  BFD_RELOC_MMIX_CBRANCH_3,

/* These are relocations for the PUSHJ instruction.  */
  BFD_RELOC_MMIX_PUSHJ,
  BFD_RELOC_MMIX_PUSHJ_1,
  BFD_RELOC_MMIX_PUSHJ_2,
  BFD_RELOC_MMIX_PUSHJ_3,
  BFD_RELOC_MMIX_PUSHJ_STUBBABLE,

/* These are relocations for the JMP instruction.  */
  BFD_RELOC_MMIX_JMP,
  BFD_RELOC_MMIX_JMP_1,
  BFD_RELOC_MMIX_JMP_2,
  BFD_RELOC_MMIX_JMP_3,

/* This is a relocation for a relative address as in a GETA instruction or
a branch.  */
  BFD_RELOC_MMIX_ADDR19,

/* This is a relocation for a relative address as in a JMP instruction.  */
  BFD_RELOC_MMIX_ADDR27,

/* This is a relocation for an instruction field that may be a general
register or a value 0..255.  */
  BFD_RELOC_MMIX_REG_OR_BYTE,

/* This is a relocation for an instruction field that may be a general
register.  */
  BFD_RELOC_MMIX_REG,

/* This is a relocation for two instruction fields holding a register and
an offset, the equivalent of the relocation.  */
  BFD_RELOC_MMIX_BASE_PLUS_OFFSET,

/* This relocation is an assertion that the expression is not allocated as
a global register.  It does not modify contents.  */
  BFD_RELOC_MMIX_LOCAL,

/* This is a 16 bit reloc for the AVR that stores 8 bit pc relative
short offset into 7 bits.  */
  BFD_RELOC_AVR_7_PCREL,

/* This is a 16 bit reloc for the AVR that stores 13 bit pc relative
short offset into 12 bits.  */
  BFD_RELOC_AVR_13_PCREL,

/* This is a 16 bit reloc for the AVR that stores 17 bit value (usually
program memory address) into 16 bits.  */
  BFD_RELOC_AVR_16_PM,

/* This is a 16 bit reloc for the AVR that stores 8 bit value (usually
data memory address) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_LO8_LDI,

/* This is a 16 bit reloc for the AVR that stores 8 bit value (high 8 bit
of data memory address) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_HI8_LDI,

/* This is a 16 bit reloc for the AVR that stores 8 bit value (most high 8 bit
of program memory address) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_HH8_LDI,

/* This is a 16 bit reloc for the AVR that stores 8 bit value (most high 8 bit
of 32 bit value) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_MS8_LDI,

/* This is a 16 bit reloc for the AVR that stores negated 8 bit value
(usually data memory address) into 8 bit immediate value of SUBI insn.  */
  BFD_RELOC_AVR_LO8_LDI_NEG,

/* This is a 16 bit reloc for the AVR that stores negated 8 bit value
(high 8 bit of data memory address) into 8 bit immediate value of
SUBI insn.  */
  BFD_RELOC_AVR_HI8_LDI_NEG,

/* This is a 16 bit reloc for the AVR that stores negated 8 bit value
(most high 8 bit of program memory address) into 8 bit immediate value
of LDI or SUBI insn.  */
  BFD_RELOC_AVR_HH8_LDI_NEG,

/* This is a 16 bit reloc for the AVR that stores negated 8 bit value (msb
of 32 bit value) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_MS8_LDI_NEG,

/* This is a 16 bit reloc for the AVR that stores 8 bit value (usually
command address) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_LO8_LDI_PM,

/* This is a 16 bit reloc for the AVR that stores 8 bit value (high 8 bit
of command address) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_HI8_LDI_PM,

/* This is a 16 bit reloc for the AVR that stores 8 bit value (most high 8 bit
of command address) into 8 bit immediate value of LDI insn.  */
  BFD_RELOC_AVR_HH8_LDI_PM,

/* This is a 16 bit reloc for the AVR that stores negated 8 bit value
(usually command address) into 8 bit immediate value of SUBI insn.  */
  BFD_RELOC_AVR_LO8_LDI_PM_NEG,

/* This is a 16 bit reloc for the AVR that stores negated 8 bit value
(high 8 bit of 16 bit command address) into 8 bit immediate value
of SUBI insn.  */
  BFD_RELOC_AVR_HI8_LDI_PM_NEG,

/* This is a 16 bit reloc for the AVR that stores negated 8 bit value
(high 6 bit of 22 bit command address) into 8 bit immediate
value of SUBI insn.  */
  BFD_RELOC_AVR_HH8_LDI_PM_NEG,

/* This is a 32 bit reloc for the AVR that stores 23 bit value
into 22 bits.  */
  BFD_RELOC_AVR_CALL,

/* This is a 16 bit reloc for the AVR that stores all needed bits
for absolute addressing with ldi with overflow check to linktime  */
  BFD_RELOC_AVR_LDI,

/* This is a 6 bit reloc for the AVR that stores offset for ldd/std
instructions  */
  BFD_RELOC_AVR_6,

/* This is a 6 bit reloc for the AVR that stores offset for adiw/sbiw
instructions  */
  BFD_RELOC_AVR_6_ADIW,

/* Direct 12 bit.  */
  BFD_RELOC_390_12,

/* 12 bit GOT offset.  */
  BFD_RELOC_390_GOT12,

/* 32 bit PC relative PLT address.  */
  BFD_RELOC_390_PLT32,

/* Copy symbol at runtime.  */
  BFD_RELOC_390_COPY,

/* Create GOT entry.  */
  BFD_RELOC_390_GLOB_DAT,

/* Create PLT entry.  */
  BFD_RELOC_390_JMP_SLOT,

/* Adjust by program base.  */
  BFD_RELOC_390_RELATIVE,

/* 32 bit PC relative offset to GOT.  */
  BFD_RELOC_390_GOTPC,

/* 16 bit GOT offset.  */
  BFD_RELOC_390_GOT16,

/* PC relative 16 bit shifted by 1.  */
  BFD_RELOC_390_PC16DBL,

/* 16 bit PC rel. PLT shifted by 1.  */
  BFD_RELOC_390_PLT16DBL,

/* PC relative 32 bit shifted by 1.  */
  BFD_RELOC_390_PC32DBL,

/* 32 bit PC rel. PLT shifted by 1.  */
  BFD_RELOC_390_PLT32DBL,

/* 32 bit PC rel. GOT shifted by 1.  */
  BFD_RELOC_390_GOTPCDBL,

/* 64 bit GOT offset.  */
  BFD_RELOC_390_GOT64,

/* 64 bit PC relative PLT address.  */
  BFD_RELOC_390_PLT64,

/* 32 bit rel. offset to GOT entry.  */
  BFD_RELOC_390_GOTENT,

/* 64 bit offset to GOT.  */
  BFD_RELOC_390_GOTOFF64,

/* 12-bit offset to symbol-entry within GOT, with PLT handling.  */
  BFD_RELOC_390_GOTPLT12,

/* 16-bit offset to symbol-entry within GOT, with PLT handling.  */
  BFD_RELOC_390_GOTPLT16,

/* 32-bit offset to symbol-entry within GOT, with PLT handling.  */
  BFD_RELOC_390_GOTPLT32,

/* 64-bit offset to symbol-entry within GOT, with PLT handling.  */
  BFD_RELOC_390_GOTPLT64,

/* 32-bit rel. offset to symbol-entry within GOT, with PLT handling.  */
  BFD_RELOC_390_GOTPLTENT,

/* 16-bit rel. offset from the GOT to a PLT entry.  */
  BFD_RELOC_390_PLTOFF16,

/* 32-bit rel. offset from the GOT to a PLT entry.  */
  BFD_RELOC_390_PLTOFF32,

/* 64-bit rel. offset from the GOT to a PLT entry.  */
  BFD_RELOC_390_PLTOFF64,

/* s390 tls relocations.  */
  BFD_RELOC_390_TLS_LOAD,
  BFD_RELOC_390_TLS_GDCALL,
  BFD_RELOC_390_TLS_LDCALL,
  BFD_RELOC_390_TLS_GD32,
  BFD_RELOC_390_TLS_GD64,
  BFD_RELOC_390_TLS_GOTIE12,
  BFD_RELOC_390_TLS_GOTIE32,
  BFD_RELOC_390_TLS_GOTIE64,
  BFD_RELOC_390_TLS_LDM32,
  BFD_RELOC_390_TLS_LDM64,
  BFD_RELOC_390_TLS_IE32,
  BFD_RELOC_390_TLS_IE64,
  BFD_RELOC_390_TLS_IEENT,
  BFD_RELOC_390_TLS_LE32,
  BFD_RELOC_390_TLS_LE64,
  BFD_RELOC_390_TLS_LDO32,
  BFD_RELOC_390_TLS_LDO64,
  BFD_RELOC_390_TLS_DTPMOD,
  BFD_RELOC_390_TLS_DTPOFF,
  BFD_RELOC_390_TLS_TPOFF,

/* Long displacement extension.  */
  BFD_RELOC_390_20,
  BFD_RELOC_390_GOT20,
  BFD_RELOC_390_GOTPLT20,
  BFD_RELOC_390_TLS_GOTIE20,

/* Scenix IP2K - 9-bit register number / data address  */
  BFD_RELOC_IP2K_FR9,

/* Scenix IP2K - 4-bit register/data bank number  */
  BFD_RELOC_IP2K_BANK,

/* Scenix IP2K - low 13 bits of instruction word address  */
  BFD_RELOC_IP2K_ADDR16CJP,

/* Scenix IP2K - high 3 bits of instruction word address  */
  BFD_RELOC_IP2K_PAGE3,

/* Scenix IP2K - ext/low/high 8 bits of data address  */
  BFD_RELOC_IP2K_LO8DATA,
  BFD_RELOC_IP2K_HI8DATA,
  BFD_RELOC_IP2K_EX8DATA,

/* Scenix IP2K - low/high 8 bits of instruction word address  */
  BFD_RELOC_IP2K_LO8INSN,
  BFD_RELOC_IP2K_HI8INSN,

/* Scenix IP2K - even/odd PC modifier to modify snb pcl.0  */
  BFD_RELOC_IP2K_PC_SKIP,

/* Scenix IP2K - 16 bit word address in text section.  */
  BFD_RELOC_IP2K_TEXT,

/* Scenix IP2K - 7-bit sp or dp offset  */
  BFD_RELOC_IP2K_FR_OFFSET,

/* Scenix VPE4K coprocessor - data/insn-space addressing  */
  BFD_RELOC_VPE4KMATH_DATA,
  BFD_RELOC_VPE4KMATH_INSN,

/* These two relocations are used by the linker to determine which of
the entries in a C++ virtual function table are actually used.  When
the --gc-sections option is given, the linker will zero out the entries
that are not used, so that the code for those functions need not be
included in the output.

VTABLE_INHERIT is a zero-space relocation used to describe to the
linker the inheritance tree of a C++ virtual function table.  The
relocation's symbol should be the parent class' vtable, and the
relocation should be located at the child vtable.

VTABLE_ENTRY is a zero-space relocation that describes the use of a
virtual function table entry.  The reloc's symbol should refer to the
table of the class mentioned in the code.  Off of that base, an offset
describes the entry that is being used.  For Rela hosts, this offset
is stored in the reloc's addend.  For Rel hosts, we are forced to put
this offset in the reloc's section offset.  */
  BFD_RELOC_VTABLE_INHERIT,
  BFD_RELOC_VTABLE_ENTRY,

/* Intel IA64 Relocations.  */
  BFD_RELOC_IA64_IMM14,
  BFD_RELOC_IA64_IMM22,
  BFD_RELOC_IA64_IMM64,
  BFD_RELOC_IA64_DIR32MSB,
  BFD_RELOC_IA64_DIR32LSB,
  BFD_RELOC_IA64_DIR64MSB,
  BFD_RELOC_IA64_DIR64LSB,
  BFD_RELOC_IA64_GPREL22,
  BFD_RELOC_IA64_GPREL64I,
  BFD_RELOC_IA64_GPREL32MSB,
  BFD_RELOC_IA64_GPREL32LSB,
  BFD_RELOC_IA64_GPREL64MSB,
  BFD_RELOC_IA64_GPREL64LSB,
  BFD_RELOC_IA64_LTOFF22,
  BFD_RELOC_IA64_LTOFF64I,
  BFD_RELOC_IA64_PLTOFF22,
  BFD_RELOC_IA64_PLTOFF64I,
  BFD_RELOC_IA64_PLTOFF64MSB,
  BFD_RELOC_IA64_PLTOFF64LSB,
  BFD_RELOC_IA64_FPTR64I,
  BFD_RELOC_IA64_FPTR32MSB,
  BFD_RELOC_IA64_FPTR32LSB,
  BFD_RELOC_IA64_FPTR64MSB,
  BFD_RELOC_IA64_FPTR64LSB,
  BFD_RELOC_IA64_PCREL21B,
  BFD_RELOC_IA64_PCREL21BI,
  BFD_RELOC_IA64_PCREL21M,
  BFD_RELOC_IA64_PCREL21F,
  BFD_RELOC_IA64_PCREL22,
  BFD_RELOC_IA64_PCREL60B,
  BFD_RELOC_IA64_PCREL64I,
  BFD_RELOC_IA64_PCREL32MSB,
  BFD_RELOC_IA64_PCREL32LSB,
  BFD_RELOC_IA64_PCREL64MSB,
  BFD_RELOC_IA64_PCREL64LSB,
  BFD_RELOC_IA64_LTOFF_FPTR22,
  BFD_RELOC_IA64_LTOFF_FPTR64I,
  BFD_RELOC_IA64_LTOFF_FPTR32MSB,
  BFD_RELOC_IA64_LTOFF_FPTR32LSB,
  BFD_RELOC_IA64_LTOFF_FPTR64MSB,
  BFD_RELOC_IA64_LTOFF_FPTR64LSB,
  BFD_RELOC_IA64_SEGREL32MSB,
  BFD_RELOC_IA64_SEGREL32LSB,
  BFD_RELOC_IA64_SEGREL64MSB,
  BFD_RELOC_IA64_SEGREL64LSB,
  BFD_RELOC_IA64_SECREL32MSB,
  BFD_RELOC_IA64_SECREL32LSB,
  BFD_RELOC_IA64_SECREL64MSB,
  BFD_RELOC_IA64_SECREL64LSB,
  BFD_RELOC_IA64_REL32MSB,
  BFD_RELOC_IA64_REL32LSB,
  BFD_RELOC_IA64_REL64MSB,
  BFD_RELOC_IA64_REL64LSB,
  BFD_RELOC_IA64_LTV32MSB,
  BFD_RELOC_IA64_LTV32LSB,
  BFD_RELOC_IA64_LTV64MSB,
  BFD_RELOC_IA64_LTV64LSB,
  BFD_RELOC_IA64_IPLTMSB,
  BFD_RELOC_IA64_IPLTLSB,
  BFD_RELOC_IA64_COPY,
  BFD_RELOC_IA64_LTOFF22X,
  BFD_RELOC_IA64_LDXMOV,
  BFD_RELOC_IA64_TPREL14,
  BFD_RELOC_IA64_TPREL22,
  BFD_RELOC_IA64_TPREL64I,
  BFD_RELOC_IA64_TPREL64MSB,
  BFD_RELOC_IA64_TPREL64LSB,
  BFD_RELOC_IA64_LTOFF_TPREL22,
  BFD_RELOC_IA64_DTPMOD64MSB,
  BFD_RELOC_IA64_DTPMOD64LSB,
  BFD_RELOC_IA64_LTOFF_DTPMOD22,
  BFD_RELOC_IA64_DTPREL14,
  BFD_RELOC_IA64_DTPREL22,
  BFD_RELOC_IA64_DTPREL64I,
  BFD_RELOC_IA64_DTPREL32MSB,
  BFD_RELOC_IA64_DTPREL32LSB,
  BFD_RELOC_IA64_DTPREL64MSB,
  BFD_RELOC_IA64_DTPREL64LSB,
  BFD_RELOC_IA64_LTOFF_DTPREL22,

/* Motorola 68HC11 reloc.
This is the 8 bit high part of an absolute address.  */
  BFD_RELOC_M68HC11_HI8,

/* Motorola 68HC11 reloc.
This is the 8 bit low part of an absolute address.  */
  BFD_RELOC_M68HC11_LO8,

/* Motorola 68HC11 reloc.
This is the 3 bit of a value.  */
  BFD_RELOC_M68HC11_3B,

/* Motorola 68HC11 reloc.
This reloc marks the beginning of a jump/call instruction.
It is used for linker relaxation to correctly identify beginning
of instruction and change some branches to use PC-relative
addressing mode.  */
  BFD_RELOC_M68HC11_RL_JUMP,

/* Motorola 68HC11 reloc.
This reloc marks a group of several instructions that gcc generates
and for which the linker relaxation pass can modify and/or remove
some of them.  */
  BFD_RELOC_M68HC11_RL_GROUP,

/* Motorola 68HC11 reloc.
This is the 16-bit lower part of an address.  It is used for 'call'
instruction to specify the symbol address without any special
transformation (due to memory bank window).  */
  BFD_RELOC_M68HC11_LO16,

/* Motorola 68HC11 reloc.
This is a 8-bit reloc that specifies the page number of an address.
It is used by 'call' instruction to specify the page number of
the symbol.  */
  BFD_RELOC_M68HC11_PAGE,

/* Motorola 68HC11 reloc.
This is a 24-bit reloc that represents the address with a 16-bit
value and a 8-bit page number.  The symbol address is transformed
to follow the 16K memory bank of 68HC12 (seen as mapped in the window).  */
  BFD_RELOC_M68HC11_24,

/* Motorola 68HC12 reloc.
This is the 5 bits of a value.  */
  BFD_RELOC_M68HC12_5B,

/* NS CR16C Relocations.  */
  BFD_RELOC_16C_NUM08,
  BFD_RELOC_16C_NUM08_C,
  BFD_RELOC_16C_NUM16,
  BFD_RELOC_16C_NUM16_C,
  BFD_RELOC_16C_NUM32,
  BFD_RELOC_16C_NUM32_C,
  BFD_RELOC_16C_DISP04,
  BFD_RELOC_16C_DISP04_C,
  BFD_RELOC_16C_DISP08,
  BFD_RELOC_16C_DISP08_C,
  BFD_RELOC_16C_DISP16,
  BFD_RELOC_16C_DISP16_C,
  BFD_RELOC_16C_DISP24,
  BFD_RELOC_16C_DISP24_C,
  BFD_RELOC_16C_DISP24a,
  BFD_RELOC_16C_DISP24a_C,
  BFD_RELOC_16C_REG04,
  BFD_RELOC_16C_REG04_C,
  BFD_RELOC_16C_REG04a,
  BFD_RELOC_16C_REG04a_C,
  BFD_RELOC_16C_REG14,
  BFD_RELOC_16C_REG14_C,
  BFD_RELOC_16C_REG16,
  BFD_RELOC_16C_REG16_C,
  BFD_RELOC_16C_REG20,
  BFD_RELOC_16C_REG20_C,
  BFD_RELOC_16C_ABS20,
  BFD_RELOC_16C_ABS20_C,
  BFD_RELOC_16C_ABS24,
  BFD_RELOC_16C_ABS24_C,
  BFD_RELOC_16C_IMM04,
  BFD_RELOC_16C_IMM04_C,
  BFD_RELOC_16C_IMM16,
  BFD_RELOC_16C_IMM16_C,
  BFD_RELOC_16C_IMM20,
  BFD_RELOC_16C_IMM20_C,
  BFD_RELOC_16C_IMM24,
  BFD_RELOC_16C_IMM24_C,
  BFD_RELOC_16C_IMM32,
  BFD_RELOC_16C_IMM32_C,

/* NS CRX Relocations.  */
  BFD_RELOC_CRX_REL4,
  BFD_RELOC_CRX_REL8,
  BFD_RELOC_CRX_REL8_CMP,
  BFD_RELOC_CRX_REL16,
  BFD_RELOC_CRX_REL24,
  BFD_RELOC_CRX_REL32,
  BFD_RELOC_CRX_REGREL12,
  BFD_RELOC_CRX_REGREL22,
  BFD_RELOC_CRX_REGREL28,
  BFD_RELOC_CRX_REGREL32,
  BFD_RELOC_CRX_ABS16,
  BFD_RELOC_CRX_ABS32,
  BFD_RELOC_CRX_NUM8,
  BFD_RELOC_CRX_NUM16,
  BFD_RELOC_CRX_NUM32,
  BFD_RELOC_CRX_IMM16,
  BFD_RELOC_CRX_IMM32,
  BFD_RELOC_CRX_SWITCH8,
  BFD_RELOC_CRX_SWITCH16,
  BFD_RELOC_CRX_SWITCH32,

/* These relocs are only used within the CRIS assembler.  They are not
(at present) written to any object files.  */
  BFD_RELOC_CRIS_BDISP8,
  BFD_RELOC_CRIS_UNSIGNED_5,
  BFD_RELOC_CRIS_SIGNED_6,
  BFD_RELOC_CRIS_UNSIGNED_6,
  BFD_RELOC_CRIS_SIGNED_8,
  BFD_RELOC_CRIS_UNSIGNED_8,
  BFD_RELOC_CRIS_SIGNED_16,
  BFD_RELOC_CRIS_UNSIGNED_16,
  BFD_RELOC_CRIS_LAPCQ_OFFSET,
  BFD_RELOC_CRIS_UNSIGNED_4,

/* Relocs used in ELF shared libraries for CRIS.  */
  BFD_RELOC_CRIS_COPY,
  BFD_RELOC_CRIS_GLOB_DAT,
  BFD_RELOC_CRIS_JUMP_SLOT,
  BFD_RELOC_CRIS_RELATIVE,

/* 32-bit offset to symbol-entry within GOT.  */
  BFD_RELOC_CRIS_32_GOT,

/* 16-bit offset to symbol-entry within GOT.  */
  BFD_RELOC_CRIS_16_GOT,

/* 32-bit offset to symbol-entry within GOT, with PLT handling.  */
  BFD_RELOC_CRIS_32_GOTPLT,

/* 16-bit offset to symbol-entry within GOT, with PLT handling.  */
  BFD_RELOC_CRIS_16_GOTPLT,

/* 32-bit offset to symbol, relative to GOT.  */
  BFD_RELOC_CRIS_32_GOTREL,

/* 32-bit offset to symbol with PLT entry, relative to GOT.  */
  BFD_RELOC_CRIS_32_PLT_GOTREL,

/* 32-bit offset to symbol with PLT entry, relative to this relocation.  */
  BFD_RELOC_CRIS_32_PLT_PCREL,

/* Intel i860 Relocations.  */
  BFD_RELOC_860_COPY,
  BFD_RELOC_860_GLOB_DAT,
  BFD_RELOC_860_JUMP_SLOT,
  BFD_RELOC_860_RELATIVE,
  BFD_RELOC_860_PC26,
  BFD_RELOC_860_PLT26,
  BFD_RELOC_860_PC16,
  BFD_RELOC_860_LOW0,
  BFD_RELOC_860_SPLIT0,
  BFD_RELOC_860_LOW1,
  BFD_RELOC_860_SPLIT1,
  BFD_RELOC_860_LOW2,
  BFD_RELOC_860_SPLIT2,
  BFD_RELOC_860_LOW3,
  BFD_RELOC_860_LOGOT0,
  BFD_RELOC_860_SPGOT0,
  BFD_RELOC_860_LOGOT1,
  BFD_RELOC_860_SPGOT1,
  BFD_RELOC_860_LOGOTOFF0,
  BFD_RELOC_860_SPGOTOFF0,
  BFD_RELOC_860_LOGOTOFF1,
  BFD_RELOC_860_SPGOTOFF1,
  BFD_RELOC_860_LOGOTOFF2,
  BFD_RELOC_860_LOGOTOFF3,
  BFD_RELOC_860_LOPC,
  BFD_RELOC_860_HIGHADJ,
  BFD_RELOC_860_HAGOT,
  BFD_RELOC_860_HAGOTOFF,
  BFD_RELOC_860_HAPC,
  BFD_RELOC_860_HIGH,
  BFD_RELOC_860_HIGOT,
  BFD_RELOC_860_HIGOTOFF,

/* OpenRISC Relocations.  */
  BFD_RELOC_OPENRISC_ABS_26,
  BFD_RELOC_OPENRISC_REL_26,

/* H8 elf Relocations.  */
  BFD_RELOC_H8_DIR16A8,
  BFD_RELOC_H8_DIR16R8,
  BFD_RELOC_H8_DIR24A8,
  BFD_RELOC_H8_DIR24R8,
  BFD_RELOC_H8_DIR32A16,

/* Sony Xstormy16 Relocations.  */
  BFD_RELOC_XSTORMY16_REL_12,
  BFD_RELOC_XSTORMY16_12,
  BFD_RELOC_XSTORMY16_24,
  BFD_RELOC_XSTORMY16_FPTR16,

/* Infineon Relocations.  */
  BFD_RELOC_XC16X_PAG,
  BFD_RELOC_XC16X_POF,
  BFD_RELOC_XC16X_SEG,
  BFD_RELOC_XC16X_SOF,

/* Relocations used by VAX ELF.  */
  BFD_RELOC_VAX_GLOB_DAT,
  BFD_RELOC_VAX_JMP_SLOT,
  BFD_RELOC_VAX_RELATIVE,

/* Morpho MT - 16 bit immediate relocation.  */
  BFD_RELOC_MT_PC16,

/* Morpho MT - Hi 16 bits of an address.  */
  BFD_RELOC_MT_HI16,

/* Morpho MT - Low 16 bits of an address.  */
  BFD_RELOC_MT_LO16,

/* Morpho MT - Used to tell the linker which vtable entries are used.  */
  BFD_RELOC_MT_GNU_VTINHERIT,

/* Morpho MT - Used to tell the linker which vtable entries are used.  */
  BFD_RELOC_MT_GNU_VTENTRY,

/* Morpho MT - 8 bit immediate relocation.  */
  BFD_RELOC_MT_PCINSN8,

/* msp430 specific relocation codes  */
  BFD_RELOC_MSP430_10_PCREL,
  BFD_RELOC_MSP430_16_PCREL,
  BFD_RELOC_MSP430_16,
  BFD_RELOC_MSP430_16_PCREL_BYTE,
  BFD_RELOC_MSP430_16_BYTE,
  BFD_RELOC_MSP430_2X_PCREL,
  BFD_RELOC_MSP430_RL_PCREL,

/* IQ2000 Relocations.  */
  BFD_RELOC_IQ2000_OFFSET_16,
  BFD_RELOC_IQ2000_OFFSET_21,
  BFD_RELOC_IQ2000_UHI16,

/* Special Xtensa relocation used only by PLT entries in ELF shared
objects to indicate that the runtime linker should set the value
to one of its own internal functions or data structures.  */
  BFD_RELOC_XTENSA_RTLD,

/* Xtensa relocations for ELF shared objects.  */
  BFD_RELOC_XTENSA_GLOB_DAT,
  BFD_RELOC_XTENSA_JMP_SLOT,
  BFD_RELOC_XTENSA_RELATIVE,

/* Xtensa relocation used in ELF object files for symbols that may require
PLT entries.  Otherwise, this is just a generic 32-bit relocation.  */
  BFD_RELOC_XTENSA_PLT,

/* Xtensa relocations to mark the difference of two local symbols.
These are only needed to support linker relaxation and can be ignored
when not relaxing.  The field is set to the value of the difference
assuming no relaxation.  The relocation encodes the position of the
first symbol so the linker can determine whether to adjust the field
value.  */
  BFD_RELOC_XTENSA_DIFF8,
  BFD_RELOC_XTENSA_DIFF16,
  BFD_RELOC_XTENSA_DIFF32,

/* Generic Xtensa relocations for instruction operands.  Only the slot
number is encoded in the relocation.  The relocation applies to the
last PC-relative immediate operand, or if there are no PC-relative
immediates, to the last immediate operand.  */
  BFD_RELOC_XTENSA_SLOT0_OP,
  BFD_RELOC_XTENSA_SLOT1_OP,
  BFD_RELOC_XTENSA_SLOT2_OP,
  BFD_RELOC_XTENSA_SLOT3_OP,
  BFD_RELOC_XTENSA_SLOT4_OP,
  BFD_RELOC_XTENSA_SLOT5_OP,
  BFD_RELOC_XTENSA_SLOT6_OP,
  BFD_RELOC_XTENSA_SLOT7_OP,
  BFD_RELOC_XTENSA_SLOT8_OP,
  BFD_RELOC_XTENSA_SLOT9_OP,
  BFD_RELOC_XTENSA_SLOT10_OP,
  BFD_RELOC_XTENSA_SLOT11_OP,
  BFD_RELOC_XTENSA_SLOT12_OP,
  BFD_RELOC_XTENSA_SLOT13_OP,
  BFD_RELOC_XTENSA_SLOT14_OP,

/* Alternate Xtensa relocations.  Only the slot is encoded in the
relocation.  The meaning of these relocations is opcode-specific.  */
  BFD_RELOC_XTENSA_SLOT0_ALT,
  BFD_RELOC_XTENSA_SLOT1_ALT,
  BFD_RELOC_XTENSA_SLOT2_ALT,
  BFD_RELOC_XTENSA_SLOT3_ALT,
  BFD_RELOC_XTENSA_SLOT4_ALT,
  BFD_RELOC_XTENSA_SLOT5_ALT,
  BFD_RELOC_XTENSA_SLOT6_ALT,
  BFD_RELOC_XTENSA_SLOT7_ALT,
  BFD_RELOC_XTENSA_SLOT8_ALT,
  BFD_RELOC_XTENSA_SLOT9_ALT,
  BFD_RELOC_XTENSA_SLOT10_ALT,
  BFD_RELOC_XTENSA_SLOT11_ALT,
  BFD_RELOC_XTENSA_SLOT12_ALT,
  BFD_RELOC_XTENSA_SLOT13_ALT,
  BFD_RELOC_XTENSA_SLOT14_ALT,

/* Xtensa relocations for backward compatibility.  These have all been
replaced by BFD_RELOC_XTENSA_SLOT0_OP.  */
  BFD_RELOC_XTENSA_OP0,
  BFD_RELOC_XTENSA_OP1,
  BFD_RELOC_XTENSA_OP2,

/* Xtensa relocation to mark that the assembler expanded the
instructions from an original target.  The expansion size is
encoded in the reloc size.  */
  BFD_RELOC_XTENSA_ASM_EXPAND,

/* Xtensa relocation to mark that the linker should simplify
assembler-expanded instructions.  This is commonly used
internally by the linker after analysis of a
BFD_RELOC_XTENSA_ASM_EXPAND.  */
  BFD_RELOC_XTENSA_ASM_SIMPLIFY,

/* 8 bit signed offset in (ix+d) or (iy+d).  */
  BFD_RELOC_Z80_DISP8,

/* DJNZ offset.  */
  BFD_RELOC_Z8K_DISP7,

/* CALR offset.  */
  BFD_RELOC_Z8K_CALLR,

/* 4 bit value.  */
  BFD_RELOC_Z8K_IMM4L,
  BFD_RELOC_UNUSED 
};


enum bfd_architecture
{
  bfd_arch_unknown,   /* File arch not known.  */
  bfd_arch_obscure,   /* Arch known, not one of these.  */
  bfd_arch_m68k,      /* Motorola 68xxx */
#define bfd_mach_m68000 1
#define bfd_mach_m68008 2
#define bfd_mach_m68010 3
#define bfd_mach_m68020 4
#define bfd_mach_m68030 5
#define bfd_mach_m68040 6
#define bfd_mach_m68060 7
#define bfd_mach_cpu32  8
#define bfd_mach_mcf_isa_a_nodiv 9
#define bfd_mach_mcf_isa_a 10
#define bfd_mach_mcf_isa_a_mac 11
#define bfd_mach_mcf_isa_a_emac 12
#define bfd_mach_mcf_isa_aplus 13
#define bfd_mach_mcf_isa_aplus_mac 14
#define bfd_mach_mcf_isa_aplus_emac 15
#define bfd_mach_mcf_isa_b_nousp 16
#define bfd_mach_mcf_isa_b_nousp_mac 17
#define bfd_mach_mcf_isa_b_nousp_emac 18
#define bfd_mach_mcf_isa_b 19
#define bfd_mach_mcf_isa_b_mac 20
#define bfd_mach_mcf_isa_b_emac 21
#define bfd_mach_mcf_isa_b_float 22
#define bfd_mach_mcf_isa_b_float_mac 23
#define bfd_mach_mcf_isa_b_float_emac 24
  bfd_arch_vax,       /* DEC Vax */
  bfd_arch_i960,      /* Intel 960 */
    /* The order of the following is important.
       lower number indicates a machine type that
       only accepts a subset of the instructions
       available to machines with higher numbers.
       The exception is the "ca", which is
       incompatible with all other machines except
       "core".  */

#define bfd_mach_i960_core      1
#define bfd_mach_i960_ka_sa     2
#define bfd_mach_i960_kb_sb     3
#define bfd_mach_i960_mc        4
#define bfd_mach_i960_xa        5
#define bfd_mach_i960_ca        6
#define bfd_mach_i960_jx        7
#define bfd_mach_i960_hx        8

  bfd_arch_or32,      /* OpenRISC 32 */

  bfd_arch_sparc,     /* SPARC */
#define bfd_mach_sparc                 1
/* The difference between v8plus and v9 is that v9 is a true 64 bit env.  */
#define bfd_mach_sparc_sparclet        2
#define bfd_mach_sparc_sparclite       3
#define bfd_mach_sparc_v8plus          4
#define bfd_mach_sparc_v8plusa         5 /* with ultrasparc add'ns.  */
#define bfd_mach_sparc_sparclite_le    6
#define bfd_mach_sparc_v9              7
#define bfd_mach_sparc_v9a             8 /* with ultrasparc add'ns.  */
#define bfd_mach_sparc_v8plusb         9 /* with cheetah add'ns.  */
#define bfd_mach_sparc_v9b             10 /* with cheetah add'ns.  */
/* Nonzero if MACH has the v9 instruction set.  */
#define bfd_mach_sparc_v9_p(mach) \
  ((mach) >= bfd_mach_sparc_v8plus && (mach) <= bfd_mach_sparc_v9b \
   && (mach) != bfd_mach_sparc_sparclite_le)
/* Nonzero if MACH is a 64 bit sparc architecture.  */
#define bfd_mach_sparc_64bit_p(mach) \
  ((mach) >= bfd_mach_sparc_v9 && (mach) != bfd_mach_sparc_v8plusb)
  bfd_arch_mips,      /* MIPS Rxxxx */
#define bfd_mach_mips3000              3000
#define bfd_mach_mips3900              3900
#define bfd_mach_mips4000              4000
#define bfd_mach_mips4010              4010
#define bfd_mach_mips4100              4100
#define bfd_mach_mips4111              4111
#define bfd_mach_mips4120              4120
#define bfd_mach_mips4300              4300
#define bfd_mach_mips4400              4400
#define bfd_mach_mips4600              4600
#define bfd_mach_mips4650              4650
#define bfd_mach_mips5000              5000
#define bfd_mach_mips5400              5400
#define bfd_mach_mips5500              5500
#define bfd_mach_mips6000              6000
#define bfd_mach_mips7000              7000
#define bfd_mach_mips8000              8000
#define bfd_mach_mips9000              9000
#define bfd_mach_mips10000             10000
#define bfd_mach_mips12000             12000
#define bfd_mach_mips16                16
#define bfd_mach_mips5                 5
#define bfd_mach_mips_sb1              12310201 /* octal 'SB', 01 */
#define bfd_mach_mipsisa32             32
#define bfd_mach_mipsisa32r2           33
#define bfd_mach_mipsisa64             64
#define bfd_mach_mipsisa64r2           65
  bfd_arch_i386,      /* Intel 386 */
#define bfd_mach_i386_i386 1
#define bfd_mach_i386_i8086 2
#define bfd_mach_i386_i386_intel_syntax 3
#define bfd_mach_x86_64 64
#define bfd_mach_x86_64_intel_syntax 65
  bfd_arch_we32k,     /* AT&T WE32xxx */
  bfd_arch_tahoe,     /* CCI/Harris Tahoe */
  bfd_arch_i860,      /* Intel 860 */
  bfd_arch_i370,      /* IBM 360/370 Mainframes */
  bfd_arch_romp,      /* IBM ROMP PC/RT */
  bfd_arch_convex,    /* Convex */
  bfd_arch_m88k,      /* Motorola 88xxx */
  bfd_arch_m98k,      /* Motorola 98xxx */
  bfd_arch_pyramid,   /* Pyramid Technology */
  bfd_arch_h8300,     /* Renesas H8/300 (formerly Hitachi H8/300) */
#define bfd_mach_h8300    1
#define bfd_mach_h8300h   2
#define bfd_mach_h8300s   3
#define bfd_mach_h8300hn  4
#define bfd_mach_h8300sn  5
#define bfd_mach_h8300sx  6
#define bfd_mach_h8300sxn 7
  bfd_arch_pdp11,     /* DEC PDP-11 */
  bfd_arch_powerpc,   /* PowerPC */
#define bfd_mach_ppc           32
#define bfd_mach_ppc64         64
#define bfd_mach_ppc_403       403
#define bfd_mach_ppc_403gc     4030
#define bfd_mach_ppc_505       505
#define bfd_mach_ppc_601       601
#define bfd_mach_ppc_602       602
#define bfd_mach_ppc_603       603
#define bfd_mach_ppc_ec603e    6031
#define bfd_mach_ppc_604       604
#define bfd_mach_ppc_620       620
#define bfd_mach_ppc_630       630
#define bfd_mach_ppc_750       750
#define bfd_mach_ppc_860       860
#define bfd_mach_ppc_a35       35
#define bfd_mach_ppc_rs64ii    642
#define bfd_mach_ppc_rs64iii   643
#define bfd_mach_ppc_7400      7400
#define bfd_mach_ppc_e500      500
  bfd_arch_rs6000,    /* IBM RS/6000 */
#define bfd_mach_rs6k          6000
#define bfd_mach_rs6k_rs1      6001
#define bfd_mach_rs6k_rsc      6003
#define bfd_mach_rs6k_rs2      6002
  bfd_arch_hppa,      /* HP PA RISC */
#define bfd_mach_hppa10        10
#define bfd_mach_hppa11        11
#define bfd_mach_hppa20        20
#define bfd_mach_hppa20w       25
  bfd_arch_d10v,      /* Mitsubishi D10V */
#define bfd_mach_d10v          1
#define bfd_mach_d10v_ts2      2
#define bfd_mach_d10v_ts3      3
  bfd_arch_d30v,      /* Mitsubishi D30V */
  bfd_arch_dlx,       /* DLX */
  bfd_arch_m68hc11,   /* Motorola 68HC11 */
  bfd_arch_m68hc12,   /* Motorola 68HC12 */
#define bfd_mach_m6812_default 0
#define bfd_mach_m6812         1
#define bfd_mach_m6812s        2
  bfd_arch_z8k,       /* Zilog Z8000 */
#define bfd_mach_z8001         1
#define bfd_mach_z8002         2
  bfd_arch_h8500,     /* Renesas H8/500 (formerly Hitachi H8/500) */
  bfd_arch_sh,        /* Renesas / SuperH SH (formerly Hitachi SH) */
#define bfd_mach_sh            1
#define bfd_mach_sh2        0x20
#define bfd_mach_sh_dsp     0x2d
#define bfd_mach_sh2a       0x2a
#define bfd_mach_sh2a_nofpu 0x2b
#define bfd_mach_sh2a_nofpu_or_sh4_nommu_nofpu 0x2a1
#define bfd_mach_sh2a_nofpu_or_sh3_nommu 0x2a2
#define bfd_mach_sh2a_or_sh4  0x2a3
#define bfd_mach_sh2a_or_sh3e 0x2a4
#define bfd_mach_sh2e       0x2e
#define bfd_mach_sh3        0x30
#define bfd_mach_sh3_nommu  0x31
#define bfd_mach_sh3_dsp    0x3d
#define bfd_mach_sh3e       0x3e
#define bfd_mach_sh4        0x40
#define bfd_mach_sh4_nofpu  0x41
#define bfd_mach_sh4_nommu_nofpu  0x42
#define bfd_mach_sh4a       0x4a
#define bfd_mach_sh4a_nofpu 0x4b
#define bfd_mach_sh4al_dsp  0x4d
#define bfd_mach_sh5        0x50
  bfd_arch_alpha,     /* Dec Alpha */
#define bfd_mach_alpha_ev4  0x10
#define bfd_mach_alpha_ev5  0x20
#define bfd_mach_alpha_ev6  0x30
  bfd_arch_arm,       /* Advanced Risc Machines ARM.  */
#define bfd_mach_arm_unknown   0
#define bfd_mach_arm_2         1
#define bfd_mach_arm_2a        2
#define bfd_mach_arm_3         3
#define bfd_mach_arm_3M        4
#define bfd_mach_arm_4         5
#define bfd_mach_arm_4T        6
#define bfd_mach_arm_5         7
#define bfd_mach_arm_5T        8
#define bfd_mach_arm_5TE       9
#define bfd_mach_arm_XScale    10
#define bfd_mach_arm_ep9312    11
#define bfd_mach_arm_iWMMXt    12
  bfd_arch_ns32k,     /* National Semiconductors ns32000 */
  bfd_arch_w65,       /* WDC 65816 */
  bfd_arch_tic30,     /* Texas Instruments TMS320C30 */
  bfd_arch_tic4x,     /* Texas Instruments TMS320C3X/4X */
#define bfd_mach_tic3x         30
#define bfd_mach_tic4x         40
  bfd_arch_tic54x,    /* Texas Instruments TMS320C54X */
  bfd_arch_tic80,     /* TI TMS320c80 (MVP) */
  bfd_arch_v850,      /* NEC V850 */
#define bfd_mach_v850          1
#define bfd_mach_v850e         'E'
#define bfd_mach_v850e1        '1'
  bfd_arch_arc,       /* ARC Cores */
#define bfd_mach_arc_5         5
#define bfd_mach_arc_6         6
#define bfd_mach_arc_7         7
#define bfd_mach_arc_8         8
 bfd_arch_m32c,     /* Renesas M16C/M32C.  */
#define bfd_mach_m16c        0x75
#define bfd_mach_m32c        0x78
  bfd_arch_m32r,      /* Renesas M32R (formerly Mitsubishi M32R/D) */
#define bfd_mach_m32r          1 /* For backwards compatibility.  */
#define bfd_mach_m32rx         'x'
#define bfd_mach_m32r2         '2'
  bfd_arch_mn10200,   /* Matsushita MN10200 */
  bfd_arch_mn10300,   /* Matsushita MN10300 */
#define bfd_mach_mn10300               300
#define bfd_mach_am33          330
#define bfd_mach_am33_2        332
  bfd_arch_fr30,
#define bfd_mach_fr30          0x46523330
  bfd_arch_frv,
#define bfd_mach_frv           1
#define bfd_mach_frvsimple     2
#define bfd_mach_fr300         300
#define bfd_mach_fr400         400
#define bfd_mach_fr450         450
#define bfd_mach_frvtomcat     499     /* fr500 prototype */
#define bfd_mach_fr500         500
#define bfd_mach_fr550         550
  bfd_arch_mcore,
  bfd_arch_ia64,      /* HP/Intel ia64 */
#define bfd_mach_ia64_elf64    64
#define bfd_mach_ia64_elf32    32
  bfd_arch_ip2k,      /* Ubicom IP2K microcontrollers. */
#define bfd_mach_ip2022        1
#define bfd_mach_ip2022ext     2
 bfd_arch_iq2000,     /* Vitesse IQ2000.  */
#define bfd_mach_iq2000        1
#define bfd_mach_iq10          2
  bfd_arch_mt,
#define bfd_mach_ms1           1
#define bfd_mach_mrisc2        2
#define bfd_mach_ms2           3
  bfd_arch_pj,
  bfd_arch_avr,       /* Atmel AVR microcontrollers.  */
#define bfd_mach_avr1          1
#define bfd_mach_avr2          2
#define bfd_mach_avr3          3
#define bfd_mach_avr4          4
#define bfd_mach_avr5          5
  bfd_arch_bfin,        /* ADI Blackfin */
#define bfd_mach_bfin          1
  bfd_arch_cr16c,       /* National Semiconductor CompactRISC. */
#define bfd_mach_cr16c         1
  bfd_arch_crx,       /*  National Semiconductor CRX.  */
#define bfd_mach_crx           1
  bfd_arch_cris,      /* Axis CRIS */
#define bfd_mach_cris_v0_v10   255
#define bfd_mach_cris_v32      32
#define bfd_mach_cris_v10_v32  1032
  bfd_arch_s390,      /* IBM s390 */
#define bfd_mach_s390_31       31
#define bfd_mach_s390_64       64
  bfd_arch_openrisc,  /* OpenRISC */
  bfd_arch_mmix,      /* Donald Knuth's educational processor.  */
  bfd_arch_xstormy16,
#define bfd_mach_xstormy16     1
  bfd_arch_msp430,    /* Texas Instruments MSP430 architecture.  */
#define bfd_mach_msp11          11
#define bfd_mach_msp110         110
#define bfd_mach_msp12          12
#define bfd_mach_msp13          13
#define bfd_mach_msp14          14
#define bfd_mach_msp15          15
#define bfd_mach_msp16          16
#define bfd_mach_msp21          21
#define bfd_mach_msp31          31
#define bfd_mach_msp32          32
#define bfd_mach_msp33          33
#define bfd_mach_msp41          41
#define bfd_mach_msp42          42
#define bfd_mach_msp43          43
#define bfd_mach_msp44          44
  bfd_arch_xc16x,     /* Infineon's XC16X Series.               */
#define bfd_mach_xc16x         1
#define bfd_mach_xc16xl        2
#define bfd_mach_xc16xs         3
  bfd_arch_xtensa,    /* Tensilica's Xtensa cores.  */
#define bfd_mach_xtensa        1
   bfd_arch_maxq,     /* Dallas MAXQ 10/20 */
#define bfd_mach_maxq10    10
#define bfd_mach_maxq20    20
  bfd_arch_z80,
#define bfd_mach_z80strict      1 /* No undocumented opcodes.  */
#define bfd_mach_z80            3 /* With ixl, ixh, iyl, and iyh.  */
#define bfd_mach_z80full        7 /* All undocumented instructions.  */
#define bfd_mach_r800           11 /* R800: successor with multiplication.  */
  bfd_arch_last
  };



typedef enum bfd_reloc_code_real bfd_reloc_code_real_type;


enum bfd_endian { BFD_EDNIAN_BIG, BFD_ENDIAN_LITTLE, BFD_ENDIAN_UNKNOWN };

typedef struct bfd_section *sec_ptr;

typedef struct bfd_target
{
    /* Identifies the kind of target, e.g., SunOS4, Ultrix, etc. */
    char *name;

    /* The "flavour" of a back end is a general indication about
       the contents of a file */
    enum bfd_flavour flavour;

    /* The order of bytes within the data area of a file */
    enum bfd_endian byteorder;

    /* The order of bytes within the header parts of a file */
    enum bfd_endian header_byteorder;

    /* A mask of all the flags which an executable may have set -
       from the set <<BFD_NO_FLAGS>>, <<HAS_RELOC>>, ...<<D_PAGED>> */
    flagword object_flags;

    /* A mask of all the flags which a section may have set - from
       the set <<SEC_NO_FLAGS>>, <<SEC_ALLOC>>, ...<<SET_NEVER_LOAD>> */
    flagword section_flags;

    /* The character normally found at the front of a symbol.
       (if any), perhaps `_'. */
    char symbol_leading_char;

    /* The pad character for the file names within an archive header */
    char ar_pad_char;

    /* The maximum number of characters in an archive header */
    unsigned short ar_max_namelen;

    /* Entries for byte swapping for data. These are different from the
       other entry points, since they don't take a BFD as the first argument.
       Certain other handlers could do the same */
    bfd_uint64_t    (*bfd_getx64)(const void *);
    bfd_int64_t     (*bfd_getx_signed_64)(const void *);
    void            (*bfd_putx64)(bfd_uint64_t, void *);
    bfd_vma         (*bfd_getx32)(const void *);
    bfd_signed_vma  (*bfd_getx_signed_32)(const void *);
    void            (*bfd_putx32)(bfd_vma, void *);
    bfd_vma         (*bfd_getx16)(const void *);
    bfd_signed_vma  (*bfd_getx_signed_16)(const void *);
    void            (*bfd_putx16)(bfd_vma, void *);

    /* Byte swapping for the headers */
    bfd_uint64_t    (*bfd_h_getx64)(const void *);
    bfd_int64_t     (*bfd_h_getx_signed_64)(const void *);
    void            (*bfd_h_putx64)(bfd_uint64_t, void *);
    bfd_vma         (*bfd_h_getx32)(const void *);
    bfd_signed_vma  (*bfd_h_getx_signed_32)(const void *);
    void            (*bfd_h_putx32)(bfd_vma, void *);
    bfd_vma         (*bfd_h_getx16)(const void *);
    bfd_signed_vma  (*bfd_h_getx_signed_16)(const void *);
    void            (*bfd_h_putx16)(bfd_vma, void *);

    /* Format dependent routines: these are vectors of entry points
       within the target vector structure, one for each format to check */

    /* Check the format of a file being read. Return a <<bfd_target *>> or zero */
    const struct bfd_target *(*_bfd_check_format[bfd_type_end])(bfd *);

    /* Set the format of a file being written */
    bfd_boolean (*_bfd_set_format[bfd_type_end])(bfd *);

    /* write cached information into a file being written, at <<bfd_close>> */
    bfd_boolean (*_bfd_write_contents[bfd_type_end])(bfd *);

    /* generic entry points */
#define BFD_JUMP_TABLE_GENERIC(NAME)    \
    NAME##_close_and_cleanup,   \
    NAME##_bfd_free_cached_info,   \
    NAME##_new_section_hook,   \
    NAME##_get_section_contents,   \
    NAME##_get_section_contents_in_window

    /* called when the BFD is being closed to do any necessary cleanup */
    bfd_boolean (*_close_and_cleanup)(bfd *);
    /* ask the BFD to free all cached information */
    bfd_boolean (*_bfd_free_cached_info)(bfd *);
    /* called when a new section is created */
    bfd_boolean (*_new_section_hook)(bfd *, sec_ptr);
    /* read the contents of a section */
    bfd_boolean (*_bfd_get_section_contents)
                    (bfd *, sec_ptr, void *, file_ptr, bfd_size_type);
    bfd_boolean (*_bfd_get_section_contents_in_window)
                    (bfd *, sec_ptr, bfd_window *, file_ptr, bfd_size_type);

    /* entry points to copy private data */
#define BFD_JUMP_TABLE_COPY(NAME)   \
    NAME##_bfd_copy_private_bfd_data,     \
    NAME##_bfd_merge_private_bfd_data,    \
    _bfd_generic_init_private_section_data, \
    NAME##_bfd_copy_private_section_data,  \
    NAME##_bfd_copy_private_symbol_data,   \
    NAME##_bfd_copy_private_header_data,   \
    NAME##_bfd_set_private_flags,          \
    NAME##_bfd_print_private_bfd_data

    /* called to copy BFD general private data from one object file
       to another */
    bfd_boolean (*_bfd_copy_private_bfd_data)(bfd *, bfd *);
    /* called to merge BFD general private data from one object file
       to a common output file when linking */
    bfd_boolean (*_bfd_merge_private_bfd_data)(bfd *, bfd *);
    /* called to initialize BFD private section data from one object file
       to another */
#define bfd_init_private_section_data(ibfd, isec, obfd, osec, link_info)    \
       BFD_SEND(obfd, _bfd_init_private_section_data, (ibfd, isec, obfd, osec, link_info))
    bfd_boolean (*_bfd_init_private_section_data)
        (bfd *, sec_ptr, bfd *, sec_ptr, struct bfd_link_info *);
    /* called to copy BFD private section data from one object file
       to another */
    bfd_boolean (*_bfd_copy_private_section_data)
        (bfd *, sec_ptr, bfd *, sec_ptr);
    /* called to copy BFD private symbol data from one symbol to another */
    bfd_boolean (*_bfd_copy_private_symbol_data)
        (bfd *, asymbol *, bfd *, asymbol *);
    /* called to copy BFD private header data from one object file to another */
    bfd_boolean (*_bfd_copy_private_header_data) (bfd *, bfd *);
    /* called to set private backend flags */
    bfd_boolean (*_bfd_set_private_flags) (bfd *, flagword);

    /* called to print private BFD data */
    bfd_boolean (*_bfd_print_private_bfd_data) (bfd *, void *);

    /* core file entry points */
#define BFD_JUMP_TABLE_CORE(NAME)   \
    NAME##_core_file_failing_command,      \
    NAME##_core_file_failing_signal,       \
    NAME##_core_file_matches_executable_p

    char *      (*_core_file_failing_command)(bfd *);
    int         (*_core_file_failing_signal)(bfd *);
    bfd_boolean (*_core_file_matches_executable_p)(bfd *, bfd *);

    /* archive entry points */
#define BFD_JUMP_TABLE_ARCHIVE(NAME)    \
    NAME##_slurp_armap,    \
    NAME##_slurp_extended_name_table,  \
    NAME##_construct_extended_name_table,  \
    NAME##_truncate_arname,    \
    NAME##_write_armap,        \
    NAME##_read_ar_hdr,        \
    NAME##_openr_next_archived_file,   \
    NAME##_get_elt_at_index,   \
    NAME##_generic_stat_arch_elt,  \
    NAME##_update_armap_timestamp

    bfd_boolean (*_bfd_slurp_armap)(bfd *);
    bfd_boolean (*_bfd_slurp_extended_name_table)(bfd *);
    bfd_boolean (*_bfd_construct_extended_name_table)
            (bfd *, char **, bfd_size_type *, const char **);
    void        (*_bfd_truncate_arname)(bfd *, const char *, char *);
    bfd_boolean (*write_armap)
                (bfd *, unsigned int, struct orl *, unsigned int, int);
    void *      (*_bfd_read_ar_hdr_fn)(bfd *);
    bfd *       (*openr_next_archived_file)(bfd *, bfd *);
#define bfd_get_elt_at_index(b,i) BFD_SEND(b, _bfd_get_elt_at_index, (b,i))
    bfd *       (*_bfd_get_elt_at_index)(bfd *, symindex);
    int         (*_bfd_stat_arch_elt)(bfd *, struct stat *);
    bfd_boolean (*_bfd_update_armap_timestamp)(bfd *);

    /* entry points used for symbols */
#define BFD_JUMP_TABLE_SYMBOLS(NAME)    \
    NAME##_get_symtab_upper_bound,     \
    NAME##_canonicalize_symtab,        \
    NAME##_make_empty_symbol,          \
    NAME##_print_symbol,               \
    NAME##_get_symbol_info,            \
    NAME##_bfd_is_local_label_name,    \
    NAME##_bfd_is_target_special_symbol,   \
    NAME##_get_lineno,     \
    NAME##_find_nearest_line,  \
    _bfd_generic_find_line, \
    NAME##_find_inliner_info,  \
    NAME##_bfd_make_debug_symbol,  \
    NAME##_read_minisymbols,   \
    NAME##_minisymbol_to_symbol

    long        (*_bfd_get_symbol_upper_bound)(bfd *);
    long        (*_bfd_canonicalize_symtab)
                    (bfd *, struct bfd_symbol **);
    struct bfd_symbol *
                (*_bfd_make_empty_symbol)(bfd *);
    void        (*_bfd_print_symbol)
                    (bfd *, void *, struct bfd_symbol *, bfd_print_symbol_type);
#define bfd_print_symbol(b, p, s, e) BFD_SEND(b, _bfd_print_symbol, (b, p, s, e))
    void        (*_bfd_get_symbol_info)
                    (bfd *, struct bfd_symbol *, symbol_info *);
#define bfd_get_symbol_info(b, p, e) BFD_SEND(b, _bfd_get_symbol_info, (b, p, e))
    bfd_boolean (*_bfd_is_local_label_name)(bfd *, const char *);
    bfd_boolean (*_bfd_is_target_special_symbol)(bfd *, asymbol *);
    alent *     (*_get_lineno)(bfd *, struct bfd_symbol *);
    bfd_boolean (*_bfd_find_nearest_line)
                    (bfd *, struct bfd_section *, struct bfd_symbol **, bfd_vma,
                    const char **, const char **, unsigned int *);
    bfd_boolean (*_bfd_find_line)
                    (bfd *, struct bfd_symbol **, struct bfd_symbol *,
                    const char **, unsigned int *);
    bfd_boolean (*_bfd_find_inliner_info)
                    (bfd *, const char **, const char **, unsigned int *);
    /* Back-door to allow format-aware applications to create debug symbols
       while using BFD for everything else. Currently used by the assembler
       when create COFF files */
    asymbol *   (*_bfd_make_debug_symbol)
                    (bfd *, void *, unsigned long size);
#define bfd_read_minisymbols(b, d, m, s)    \
       BFD_SEND(b, _read_minisymbols, (b, d, m, s))
    long        (*_read_minisymbols)
                    (bfd *, bfd_boolean, void **, unsigned int *);
#define bfd_minisymbol_to_symbol(b, d, m, f)    \
       BFD_SEND(b, _minisymbol_to_symbol, (b, d, m, f))
    asymbol *   (*_minisymbol_to_symbol)
                    (bfd *, bfd_boolean, const void *, asymbol *);

    /* routines for relocs */
#define BFD_JUMP_TABLE_RELOCS(NAME) \
    NAME##_get_reloc_upper_bound,  \
    NAME##_canonicalize_reloc, \
    NAME##_bfd_reloc_type_lookup

    long        (*_get_reloc_upper_bound)(bfd *, sec_ptr);
    long        (*_bfd_canonicalize_reloc)
                    (bfd *, sec_ptr, arelent **, struct bfd_symbol **);
    /* see documentation on reloc types */
    reloc_howto_type *
                (*reloc_type_lookup)(bfd *, bfd_reloc_code_real_type);

    /* routines used when writing an object file */
#define BFD_JUMP_TABLE_WRITE(NAME)  \
    NAME##_set_arch_mach,  \
    NAME##_set_section_contents

    bfd_boolean (*_bfd_set_arch_mach)
                    (bfd *, enum bfd_architecture, unsigned long);
    bfd_boolean (*_bfd_set_section_contents)
                    (bfd *, sec_ptr, const void *, file_ptr, bfd_size_type);

    /* routines used by the linker */
#define BFD_JUMP_TABLE_LINK(NAME)   \
    NAME##_sizeof_headers,     \
    NAME##_bfd_get_relocated_section_contents, \
    NAME##_bfd_relax_section,      \
    NAME##_bfd_link_hash_table_create, \
    NAME##_bfd_link_hash_table_free,   \
    NAME##_bfd_link_add_symbols,       \
    NAME##_bfd_link_just_syms,         \
    NAME##_bfd_final_link,         \
    NAME##_bfd_link_split_section, \
    NAME##_bfd_gc_sections,    \
    NAME##_bfd_merge_sections, \
    NAME##_bfd_is_group_section,   \
    NAME##_bfd_discard_group,  \
    NAME##_section_already_linked

    int         (*_bfd_sizeof_headers)(bfd *, bfd_boolean);
    bfd_byte *  (*_bfd_get_relocated_section_contents)
                    (bfd *, struct bfd_link_info *, struct bfd_link_order *,
                     bfd_byte *, bfd_boolean, struct bfd_symbol **);

    /* create a hash table for the linker. Different backends store
       different information in this table */
    struct bfd_link_hash_table *
                (*_bfd_link_hash_table_create)(bfd *);
    /* release the memory associated with the linker hash table */
    void        (*_bfd_link_hash_table_free)(struct bfd_link_hash_table *);
    /* add symbols from this object file into the hash table */
    bfd_boolean (*_bfd_link_add_symbols)(bfd *, struct bfd_link_info *);
    /* indicate that we are only retrieving symbol values from this section */
    void        (*_bfd_link_just_syms)(asection *, struct bfd_link_info *);
    /* do a link based on the link_order structures attached to each
       section of the BFD */
    bfd_boolean (*_bfd_final_link)(bfd *, struct bfd_link_info *);
    /* should this section be split up into smaller pieces during linking */
    bfd_boolean (*_bfd_link_split_section)(bfd *, struct bfd_section *);
    /* remove sections that are not referenced from the output */
    bfd_boolean (*_bfd_gc_sections)(bfd *, struct bfd_link_info *);
    /* attempt to merge SEC_MERGE sections */
    bfd_boolean (*_bfd_merge_sections)(bfd *, struct bfd_link_info *);
    /* is this section a member of a group ? */
    bfd_boolean (*_bfd_is_group_section)(bfd *, const struct bfd_section *);
    /* discard members of a group */
    bfd_boolean (*_bfd_discard_group)(bfd *, struct bfd_section *);
    /* check if SEC has been already linked during a reloceatable or
       final link */
    void        (*_section_already_linked)(bfd *, struct bfd_section *);

    /* routines to handle dynamic symbols and relocs */
#define BFD_JUMP_TABLE_DYNAMIC(NAME)    \
    NAME##_get_dynamic_symtab_upper_bound, \
    NAME##_canonicalize_dynamic_symtab,    \
    NAME##_get_synthetic_symtab,   \
    NAME##_get_dynamic_reloc_upper_bound,  \
    NAME##_canonicalize_dynamic_reloc

    /* get the amount of memory required to hold the dynamic symbols */
    long        (*_bfd_get_dynamic_symtab_upper_bound)(bfd *);
    /* read in the dynamic symbols */
    long        (*_bfd_canonicalize_dynamic_symtab)
                    (bfd *, struct bfd_symbol **);
    /* create synthetized symbols */
    long        (*_bfd_get_synthetic_symtab)
                    (bfd *, long, struct bfd_symbol **, 
                     struct bfd_symbol **, struct bfd_symbol **);
    /* get the amount of memory required to hold the dynamic relocs */
    long        (*_bfd_get_dynamic_reloc_upper_bound)(bfd *);
    /* read in the dynamic relocs */
    long        (*_bfd_canonicalize_dynamic_reloc)
                    (bfd *, arelent **, struct bfd_symbol **);

    /* opposite endian version of this target */
    const struct bfd_target * alternative_target;

    /* data for use by back-end routines, which isn't
       generic enough to belong in this structure */
    const void *backend_data;
} bfd_target;


const char **bfd_target_list(void);



#endif
