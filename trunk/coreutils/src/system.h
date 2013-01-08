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



#define HELP_OPTION_DESCRIPTION \
    _("         --help      display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
    _("         --version   output version information and exit\n")


#endif // __CONFIG_H
