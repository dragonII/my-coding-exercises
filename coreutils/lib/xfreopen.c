/* a wrapper for freopen */

#include "xfreopen.h"

#include <errno.h>
#include <error.h>
#include <stdio.h>

#include "exitfail.h"

#include "gettext.h"
#define _(msgid) gettext(msgid)

void xfreopen(char* filename, char* mode, FILE* fp)
{
    if(!freopen(filename, mode, fp))
    {
        const char* f = (filename ? filename
                    : (fp == stdin ? _("stdin")
                        : (fp == stdout ? _("stdout")
                            : (fp == stderr ? _("stderr")
                                : _("unknown stream")))));
        error(exit_failure, errno, _("failed to reopen %s with mode %s"),
                f, mode);
    }
}
