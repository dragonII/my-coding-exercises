#ifndef __CONFIG_H
#define __CONFIG_H


#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

#include <locale.h>

#include "progname.h"
#include "config.h"
#include "version-etc.h"

/* ISDIGIT differs from isdigit, as follows:
    - Its arg may be any int or unsigned int; it need not be an unsigned char
      or EOF.
    - It's typically faster.
   POSIX says that only '0' through '9' are digits. Prefer ISDIGIT to
   isdigit unless it's important to use the locale's definition
   of `digit' even when the host does not conform to POSIX */
#define ISDIGIT(c) ((unsigned int) (c) - '0' <= 9)

/* Take care of NLS matters. (Native Language String?)*/

#include "gettext.h"
#if ! ENABLE_NLS
# undef textdomain
# define textdomain(Domainname) /* empty */
# undef bindtextdomain
# define bindtextdomain(Domainname, Dirname) /* empty */
#endif /* ! ENABLE_NLS */

#define _(msgid) gettext (msgid)
#define N_(msgid) msgid

#include <errno.h>
/* Some systems don't define this; POSIX mentions it but says it is
   obsolete, so gnulib does not provide it either. */
#ifndef ENODATA
# define ENODATA (-1)
#endif

#include <stdbool.h>
#include <stdlib.h>
#include "version.h"

/* Exit statuses for program like 'env' that exec other programs. */
enum
{
    EXIT_TIMEDOUT       = 124,      /* Time expired before child completed */
    EXIT_CANCELED       = 125,      /* Internal error piror to exec attempt */
    EXIT_CANNOT_INVOKE  = 126,      /* Program located, but not usable */
    EXIT_ENOENT         = 127       /* Could not find program to exec */
};

#include "exitfail.h"

static inline void
initialize_exit_failure(int status)
{
    if(status != EXIT_FAILURE)
        exit_failure = status;
}

/* Return a value that pluralizes the same way that N does, in all
   languages we know of. */
static inline unsigned long int
select_plural(uintmax_t n)
{
    /* Reduce by a power of ten, but keep it away from zero. The
       gettext manual says 100,0000 should be safe. */
    enum { PLURAL_REDECER = 1000000 };
    return ( n <= ULONG_MAX ? n : n % PLURAL_REDECER + PLURAL_REDECER);
}

extern const char* program_name;
extern char* last_component(char const* name);

static inline void
emit_ancillary_info()
{
    printf(_("\nReport %s bugs to %s\n"), last_component(program_name),
            PACKAGE_BUGREPORT);

    printf(_("%s home page: <http://www.gnu.org/software/%s/>\n"),
            PACKAGE_NAME, PACKAGE);

    fputs(_("General help using GNU software: <http://www.gnu.org/gethelp/>\n"),
            stdout);

    const char *lc_message = setlocale(LC_MESSAGES, NULL);
    if(lc_message && strncmp(lc_message, "en_", 3))
    {
        printf(_("Report %s translation bugs to "
                    "<http://translationproject.org/team/>\n"),
                    last_component(program_name));
    }
    printf(_("For complete documentation, run: "
            "info coreutils '%s invocation'\n"), 
            last_component(program_name));
}

/* Redirection and wildcarding when done by the utility itself.
   Generally a noop, but used in particular for native VMS.*/
#ifndef initialize_main
# define initialize_main(ac, av)
#endif

static inline char* bad_cast(char* s)
{
    return (char*)s;
}

/* Factor out some of the common --help and --version processing code. */

/* These enum values cannot possibly conflict with the option values
   ordinarily used by commands, including CHAR_MAX + 1, etc. Avoid
   CHAR_MIN - 1, as it may equal -1, the getopt end-of-options values. */

enum
{
    GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
};

#define GETOPT_HELP_OPTION_DECL \
    "help", no_argument, NULL, GETOPT_HELP_CHAR
#define GETOPT_VERSION_OPTION_DECL \
    "version", no_argument, NULL, GETOPT_VERSION_CHAR
#define GETOPT_SELINUX_CONTEXT_OPTION_DECL \
    "context", required_argument, NULL, 'Z'

#define case_GETOPT_HELP_CHAR   \
    case GETOPT_HELP_CHAR:      \
        usage(EXIT_SUCCESS);    \
        break;

#define case_GETOPT_VERSION_CHAR(Program_name, Authors) \
    case GETOPT_VERSION_CHAR:   \
        version_etc(stdout, Program_name, PACKAGE_NAME, Version, Authors, \
                    (char*)NULL); \
        exit(EXIT_SUCCESS);         \
        break;

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* Program_name must be a literal string.
   Usually it is just PROGRAM_NAME. */
#define USAGE_BUILTIN_WARNING \
  _("\n" \
"NOTE: your shell may have its own version of %s, which usually supersedes\n" \
"the version described here. Please refer to your shell's documentation\n" \
"for details about the options it supports.\n")

#define HELP_OPTION_DESCRIPTION \
    _("         --help      display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
    _("         --version   output version information and exit\n")


#endif // __CONFIG_H
