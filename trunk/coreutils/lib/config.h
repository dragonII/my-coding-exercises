#ifndef _LIB_CONFIG_H
#define _LIB_CONFIG_H

#define PACKAGE_BUGREPORT   "bug-coreutils@gnu.org"
#define PACKAGE_NAME        "GNU coreutils"
#define PACKAGE             "coreutils"


/* Define to 1 if you have the `sync' function. */
#define HAVE_SYNC 1

/* Define to the type of elements in the array set by `getgroups'. Usually
   this is either `int' or  `gid_t'; */
#define GETGROUPS_T gid_t

/* Define to 1 if O_NOFOLLOW works */
#define HAVE_WORKING_O_NOFOLLOW 1

/* Define to the type that is the result of default argument promotions of
   type mode_t */
#define PROMOTED_MODE_T mode_t

/* Define to 1 if you have the `lchmod' function */
/* #undef HAVE_LCHMOD */

/* Define to `unsigned int' if <sys/types.h> does not define. */
#define major_t unsigned int

/* Define to `unsigned int' if <sys/types.h> does not define. */
#define minor_t unsigned int


#endif
