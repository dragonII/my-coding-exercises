#ifndef _MAGIC__H
#define _MAGIC__H

#include <sys/types.h>

#define MAGIC_NONE              0x000000    /* no flags */
#define MAGIC_DEBUG             0x000001    /* turn on debugging */
#define MAGIC_SYMLINK           0x000002    /* follow symlinks */
#define MAGIC_COMPRESS          0x000004    /* check inside compressed files */
#define MAGIC_DEVICES           0x000008    /* look at the contents of devices */
#define MAGIC_MIME_TYPE         0x000010    /* return the MIME type */
#define MAGIC_CONTINUE          0x000020    /* return all matches */
#define MAGIC_CHECK             0x000040    /* print warnings to stderr */
#define MAGIC_PRESERVE_ATIME    0x000080    /* restore access time on exit */
#define MAGIC_RAW               0x000100    /* Don't translate unprintable chars */
#define MAGIC_ERROR             0x000200    /* handle ENOENT etc as real errors */
#define MAGIC_MIME_ENCODING     0x000400    /* return the MIME encoding */
#define MAGIC_MIME              (MAGIC_MIME_TYPE|MAGIC_MIME_ENCODING)
#define MAGIC_APPLE             0x000800    /* return the Apple creator and type */

#define MAGIC_NO_CHECK_COMPRESS     0x001000    /* don't check for compressed files */
#define MAGIC_NO_CHECK_TAR          0x002000    /* don't check for tar files */
#define MAGIC_NO_CHECK_SOFT         0x004000    /* don't check magic entries */
#define MAGIC_NO_CHECK_APPTYPE      0x008000    /* don't check application type */
#define MAGIC_NO_CHECK_ELF          0x010000    /* don't check for elf details */
#define MAGIC_NO_CHECK_TEXT         0x020000    /* don't check for text files */
#define MAGIC_NO_CHECK_CDF          0x040000    /* don't check for cdf files */
#define MAGIC_NO_CHECK_TOKENS       0x100000    /* don't check tokens */
#define MAGIC_NO_CHECK_ENCODING     0x200000    /* don't check text encodings */

/* No built-in tests; only consult the magic file */
#define MAGIC_NO_CHECK_BUILTIN  (   \
    MAGIC_NO_CHECK_COMPRESS |   \
    MAGIC_NO_CHECK_TAR      |   \
    MAGIC_NO_CHECK_APPTYPE  |   \
    MAGIC_NO_CHECK_ELF      |   \
    MAGIC_NO_CHECK_TEXT     |   \
    MAGIC_NO_CHECK_CDF      |   \
    MAGIC_NO_CHECK_TOKENS   |   \
    MAGIC_NO_CHECK_ENCODING |   \
)

/* Defined for backwards compatibility (renamed) */
#define MAGIC_NO_CHECK_ASCII    MAGIC_NO_CHECK_TEXT

/* Defined for backwards compatibility; do nothing */
#define MAGIC_NO_CHECK_FORTRAN  0x000000    /* don't check ascii/fortran */
#define MAGIC_NO_CHECK_TROFF    0x000000    /* don't check ascii/troff */

#define MAGIC_VERSION   "X.YY"    /* This implementation */
#define VERSION         "X.YY"   




#endif
