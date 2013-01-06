#ifndef _VERSION_ETC_H
#define _VERSION_ETC_H

/* Display the --version information the standard way.
   Authos names are given in the NULL-teminated va_list AUTHORS */
void version_etc_va(FILE* stream,
                    const char *command_name, const char* package,
                    const char* version, va_list authors);


#endif
