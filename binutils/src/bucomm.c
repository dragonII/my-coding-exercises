/* bucomm.c -- Bin Utils COmmon code.

   This file is part of GNU Binutils.

 */

#include "bucomm.h"

#include <stdio.h>
#include <stdlib.h>

char *program_name;


/* List the supported targets */
void list_supported_targets(const char *name, FILE *f)
{
    int t;
    const char **targ_names = bfd_target_list();

    if(name == NULL)
        fprintf(f, _("Supported targets: "));
    else
        fprintf(f, _("%s: supported targets:"), name);

    for(t = 0; targ_names[t] != NULL; t++)
        fprintf(f, " %s", targ_names[t]);
    fprintf(f, "\n");
    free(targ_names);
}
