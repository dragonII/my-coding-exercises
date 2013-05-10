#include "include/bfd_.h"

const bfd_target a_out_adobe_vec =
{
  "a.out.adobe",		/* Name.  */
  bfd_target_aout_flavour,
  BFD_ENDIAN_BIG,		/* Data byte order is unknown (big assumed).  */
  BFD_ENDIAN_BIG,		/* Header byte order is big.  */
  (HAS_RELOC | EXEC_P |		/* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | WP_TEXT ),
  /* section flags */
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_DATA | SEC_RELOC),
  '_',				/* Symbol leading char.  */
  ' ',				/* AR_pad_char.  */
  16,				/* AR_max_namelen.  */

  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* Data.  */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* Headers.  */

  {_bfd_dummy_target, aout_adobe_object_p,	/* bfd_check_format.  */
   bfd_generic_archive_p, _bfd_dummy_target},
  {bfd_false, aout_adobe_mkobject,		/* bfd_set_format.  */
   _bfd_generic_mkarchive, bfd_false},
  {bfd_false, aout_adobe_write_object_contents,/* bfd_write_contents.  */
   _bfd_write_archive_contents, bfd_false},

  BFD_JUMP_TABLE_GENERIC (aout_32),
  BFD_JUMP_TABLE_COPY (_bfd_generic),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_bsd),
  BFD_JUMP_TABLE_SYMBOLS (aout_32),
  BFD_JUMP_TABLE_RELOCS (aout_32),
  BFD_JUMP_TABLE_WRITE (aout_32),
  BFD_JUMP_TABLE_LINK (aout_32),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL
};
