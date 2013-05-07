/* Functions declarations for libiberty 

   The libiberty library provides a number of functions which are
   missing on some operating systems. We do not declare those here,
   to avoid conflicts with the sytem header files on operating 
   systems that do support those functions. In this file we only
   declare those functions which are specific to libiberty */

#ifndef LIBIBERTY_H
#define LIBIBERTY_H

/* set the program name used by xmalloc */
void xmalloc_set_program_name(const char *);

/* expand arguments in argv */
void expandargv(int *, char ***);


#endif
