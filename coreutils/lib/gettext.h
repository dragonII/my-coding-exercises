#ifndef _LIBGETTEXT_H
#define _LIBGETTEXT_H

/* NLS can be disabled through the configure --disable-nls option */
#if ENABLE_NLS

#include <libintl.h>

/* You can set the DEFAULT_TEXT_DOMAIN macro to specify the domain used by
   the gettext() and ngettext() macros. This is an alternative to calling
   textdomain(), and is useful for libraries. */

# ifdef DEFAULT_TEXT_DOMAIN
#  undef gettext
#  define gettext(Msgid) dgettext(DEFAULT_TEXT_DOMAIN, Msgid)
#  undef ngettext
#  define ngettext(Msgid1, Msgid2, N) \
            dngettext(DEFAULT_TEXT_DOMAIN, Msgid1, Msgid2, N)
# endif
#else  /* not defined ENABLE_NLS */

#undef textdomain
#define textdomain(Domainname) ((const char*) (Domainname))

#undef bindtextdomain
#define bindtextdomain(Domainname, Dirname) \
            ((void)(Domainname), (const char*)(Dirname))


#endif /*ENABLE_NLS*/


#endif /* _LIBGETTEXT_H */
