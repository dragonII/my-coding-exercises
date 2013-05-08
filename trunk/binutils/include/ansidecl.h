/* ANSI and traditional C compatability macros */

#ifndef _ANSIDECL_H
#define _ANSIDECL_H

/* variadic function helper macros */
/* "struct Qdmy" swallows the semicolon after VA_OPEN/VA_FIXEDARG's
   use without inhibiting further decls and without declaring an
   actual variable */
#define VA_OPEN(AP, VAR)    { va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)        } va_end(AP); }
#define VA_FIXEDARG(AP, T, N)   struct Qdmy


#endif
