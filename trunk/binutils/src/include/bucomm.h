/* bucomm.h -- binutils common include file.
  
   This file is part of GNU Binutils.

 */
#ifndef _BUCOMM_H
#define _BUCOMM_H

#include <libintl.h>
#ifndef _
#define _(string)   gettext(string)
#endif

#include <stdio.h>

extern char *program_name;

void list_supported_targets(const char *name, FILE *f);

#endif
