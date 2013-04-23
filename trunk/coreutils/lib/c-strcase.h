#ifndef C_STRCASE_H
#define C_STRCASE_H


#include <stddef.h>

/* The functions defined in this file assume the "C" locale and a character
   set without diacritics (ASCII-US or EBCDIC-US or something like that).
   Even if the "C" locale on a particular system is an extension of the ASCII
   character set (like on BeOS, where it is UTF-8, or on AmigaOS, where it
   is ISO-8859-1), the functions in this file recognize only the ASCII
   characters. More precisely, one of the string argument must be an ASCII
   string; the other one can also contain non-ASCII characters (but then
   the comparison result will be nonzero). */

/* Compare string S1 and S2, ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less than, equal to or greater
   than S2 */
int c_strcasecmp(const char *s1, const char *s2);


#endif
