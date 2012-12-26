#include <stdio.h>

static inline void
emit_ancillary_info()
{
    printf(_("\nReport %s bugs to %s\n"), last_component(program_name),
            PACKAGE_BUGREPORT);

    printf(_("%s home page: <http://www.gnu.org/software/%s/>\n"),
            PACKAGE_NAME, PACKAGE);

    fputs(_("General help using GNU software: <http://www.gnu.org/gethelp/>\n"),
            stdout);

    const char *lc_message = setlocale(LC_MESSAGE, NULL);
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
