/* Main header file for the bfd library -- portable access to object files */

#ifndef __BFD_H__
#define __BFD_H__

typedef long long bfd_int64_t;
typedef unsigned long long bfd_uint64_t;
typedef unsigned long long bfd_size_type
typedef unsigned int flagword;  /* 32 bits of flags */
typedef unsigned char bfd_byte;
typedef unsigned long long bfd_vma;
typedef long long bfd_signed_vma;

/* file formats */
typedef enum bfd_format
{
    bfd_unknown = 0,    /* file format is unknown */
    bfd_object,         /* linker/assembler/compiler output */
    bfd_archive,        /* object archive file */
    bfd_core,           /* core dump */
    bfd_type_end
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

enum bfd_endian { BFD_EDNIAN_BIG, BFD_ENDIAN_LITTLE, BFD_ENDIAN_UNKNOWN };

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
        (bfd *, sec_ptr, bfd *, sec_ptr, struct bfd_link *);
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
    bfd_boolean (*_bfd_update_armap, timestamp)(bfd *);

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
                (*_bfd_make_debug_symbol)(bfd *);
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
                (*reloc_type_lookup)(bfd *, bfd_reloc_code_read_type);


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




#endif
