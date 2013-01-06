#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

#include <locale.h>

#include "progname.h"
#include "config.h"

#define HELP_OPTION_DESCRIPTION \
    _("         --help      display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
    _("         --version   output version information and exit\n")

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
