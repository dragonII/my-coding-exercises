/* bucomm.h -- binutils common include file.
  
   This file is part of GNU Binutils.

 */
#ifndef _BUCOMM_H
#define _BUCOMM_H

/* configured target name */
#define TARGET "i686-pc-linux-gnu"

#include <libintl.h>
#ifndef _
#define _(string)   gettext(string)
#endif

#include <stdio.h>
#include <sys/types.h>

extern char *program_name;

void list_supported_targets(const char *, FILE *);
void list_matching_formats(char **);
void set_default_bfd_target(void);

void non_fatal (const char *, ...);
void fatal(const char *, ...);
void bfd_nonfatal(const char *);

void print_version(const char *);

off_t get_file_size(const char *);


#endif
