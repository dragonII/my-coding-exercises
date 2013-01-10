#ifndef DIRNAME_H_
# define DIRNAME_H_

# ifndef DIRECTORY_SEPARATOR
#  define DIRECTORY_SEPARATOR '/'
# endif

# ifndef ISSLASH
#  define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
# endif

# ifndef FILE_SYSTEM_PREFIX_LEN
#  if FILE_SYSTEM_ACCEPTS_DRIVE_LETTER_PREFIX
    /* This internal macro assumes ASCII, but all hosts that support drive
       letters use ASCII. */
#   define _IS_DRIVE_LETTER(c) (((unsigned int) (c) | ('a' - 'A')) - 'a' <= 'z' - 'a')
#   define FILE_SYSTEM_PREFIX_LEN(Filename) \
                (_IS_DRIVE_LETTER ((Filename)[0]) && (Filename)[1] == ':' ? 2 : 0)
#  else
#   define FILE_SYSTEM_PREFIX_LEN(Filename) 0
#  endif  /*FILE_SYSTEM_ACCEPTS_DRIVE_LETTER_PREFIX*/
# endif  /*FILE_SYSTEM_PREFIX_LEN*/

# ifndef FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE
#  define FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE 0
# endif

# if FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE
#  define IS_ABSOLUTE_FILE_NAME(F) ISSLASH ((F)[FILE_SYSTEM_PREFIX_LEN])
# else
#  define IS_ABSOLUTE_FILE_NAME(F) \
            (ISSLASH ((F)[0]) || FILE_SYSTEM_PREFIX_LEN (F) > 0)
# endif

# define IS_RELATIVE_FILE_NAME(F) (! IS_ABSOLUTE_FILE_NAME(F))

# ifndef DOUBLE_SLASH_IS_DISTINCT_ROOT
#  define DOUBLE_SLASH_IS_DISTINCT_ROOT 0
# endif


#endif /*DIRNAME_H_*/
