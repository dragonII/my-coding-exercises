/* ANSI and traditional C compatability macros */

#ifndef _ANSIDECL_H
#define _ANSIDECL_H


/* For writing functions which take variable numbers of arguments, we
   provide the VA_OPEN, VA_CLOSE, and VA_FIXEDARG macros. These
   hide the differences between K+R <varargs.h> and C89 <stdarg.h> more
   thoroughly than the simple VA_START().

   VA_OPEN and VA_CLOSE are used *instead of* va_start and va_end.
   Immediately after VA_OPEN, put a sequence of VA_FIXEDARG calls
   corresponding to the list of fixed arguments. The use va_arg
   normally to get the variable arguments, or pass your va_list object
   around. You do not declare the va_list yourself; VA_OPEN does it 
   for you.

   Here is a complete example:
   
    int printf(const char *format, ...)
    {
        int result;

        VA_OPEN(ap, format);
        VA_FIXEDARG(ap, const char *, format);

        result = vfprintf(stdout, format, ap);
        VA_CLOSE(ap);

        return result;
    }

    You can declare variables either before or after the VA_OPEN,
    VA_FIXEDARG sequence. Also, VA_OPEN and VA_CLOSE are the beginning
    and end of a block. They must appear at the same nesting level,
    and any variables declared after VA_OPEN go out of scope at
    VA_CLOSE. Unfornately, with a K+R compiler, that includes the
    argument list. You can have multiple instances of VA_OPEN/VA_CLOSE
    pairs in a single function in case you need to traverse the
    argument list more than once.
 */

/* variadic function helper macros */
/* "struct Qdmy" swallows the semicolon after VA_OPEN/VA_FIXEDARG's
   use without inhibiting further decls and without declaring an
   actual variable */
#define VA_OPEN(AP, VAR)    { va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)        } va_end(AP); }
#define VA_FIXEDARG(AP, T, N)   struct Qdmy


//int printf(const char *format, ...)
//{
//    int result;
//
//    //VA_OPEN(ap, format);
//    { va_list ap; va_start(ap, format); { struct Qdmy
//    //VA_FIXEDARG(ap, const char *, format);
//      struct Qdmy
//
//      result = vfprintf(stdout, format, ap);
//    //VA_CLOSE(ap);
//    } va_end(ap); }
//
//    return result;
//}



#endif
