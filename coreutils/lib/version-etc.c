/* Print --version and bug-reporting information in a consistent format. */

#include <stdio.h>
#include <stdarg.h>

#include "version-etc-fsf.h"
#include "gettext.h"

#define _(msgid) gettext(msgid)

enum { COPYRIGHT_YEAR = 2010 };

/* The three functions below display the --version information the
   standard way.

   If COMMAND_NAME is NULL, the PACKAGE is assumed to be the name of 
   the program. The formats are therefor:

   PACKAGE VERSION

   or

   COMMAND_NAME (PACKAGE) VERSION.

   The functions differ in the way they are passed author names. */

/* Display the --version information the standard way 
   Author names are given in the array AUTHORS. N_AUTHORS is the
   number of the elements in the array. */
void version_etc_arn(FILE* stream,
                     const char* command_name, const char* package,
                     const char* version,
                     const char** authors, size_t n_authors)
{
    if(command_name)
        fprintf(stream, "%s (%s) %s\n", command_name, package, version);
    else
        fprintf(stream, "%s %s\n", package, version);

    fprintf(stream, version_etc_copyright, _("(C)"), COPYRIGHT_YEAR);

    fputs(_("\
\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
"),
            stream);
    if(n_authors == 1)
        fprintf(stream, _("Written by %s.\n"), authors[0]);
}

/* Display the --version information the standard way.
   Authos names are given in the NULL-teminated va_list AUTHORS */
void version_etc_va(FILE* stream,
                    const char *command_name, const char* package,
                    const char* version, va_list authors)
{
    size_t n_authors;
    char* authtab[10];

    for(n_authors = 0; 
        n_authors < 10 && (authtab[n_authors] = va_arg(authors, char*)) != NULL;
        n_authors++)
        ;

    version_etc_arn(stream, command_name, package, version,
                    (const char**)authtab, n_authors);
}

/* Display the --version information the standard way.

   If COMMAND_NAME is NULL, the PACKAGE is assumed to be the name of
   the program. The formats are therefore:

   PACKAGE VERSION

   or

   COMMAND_NAME (PACKAGE) VERSION.

   The authors names are passed as seperate arguments, with an additional
   NULL argument at the end. */

void version_etc(FILE* stream,
                 const char* command_name, const char* package,
                 const char* version, /*const char* author1, ...*/ ...)
{
    va_list authors;

    va_start(authors, version);
    version_etc_va(stream, command_name, package, version, authors);
    va_end(authors);
}
