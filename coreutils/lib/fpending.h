#include <stddef.h>
#include <stdio.h>

#ifndef HAVE_DECL__FPENDING
"this configure-time declaration test was not run"
#endif

#if HAVE_DECL__FPENDING
# if HAVE_STDIO_EXT_H
#  include <stdio_ext.h>
# endif
#else
size_t __fpending(FILE *);
#endif
