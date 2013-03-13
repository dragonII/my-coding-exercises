/* Determine whether string value is affirmative or negative response
   according to current locale's data */

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <regex.h>
#include "gettext.h"
#ifndef _
# define _(msgid) gettext(msgid)
# define N_(msgid) gettext_noop(msgid)
#endif


/* Return the localized regular expression pattern corresponding to
   ENGLISH_PATTERN. */
#define localized_pattern(english_pattern, nl_index, posixly_correct) \
   _(english_pattern)


static int
try(char* response, char* pattern, char** lastp, regex_t* re)
{
    if(*lastp == NULL || strcmp(pattern, *lastp) != 0)
    {
        char* safe_pattern;

        /* The pattern has changed */
        if(*lastp != NULL)
        {
            /* Free the old compiled pattern */
            regfree(re);
            free(*lastp);
            *lastp = NULL;
        }
        /* Put the PATTERN into safe memory before calling regcomp.
           (regcomp may call nl_langinfo, overwriting PATTERN's storage. */
        safe_pattern = strdup(pattern);
        if(safe_pattern == NULL)
            return -1;
        /* Compile the pattern and cache it for future runs */
        if(regcomp(re, safe_pattern, REG_EXTENDED) != 0)
            return -1;
        *lastp = safe_pattern;
    }

    /* See if the regular expression matches RESPONSE */
    return regexec(re, response, 0, NULL, 0) == 0;
}

int rpmatch(char* response)
{
    /* Match against one of the response patterns, compiling the pattern
       first if necessary. */

    /* We cache the response patterns and compiled regexpr here */
    static char *last_yesexpr, *last_noexpr;
    static regex_t cached_yesre, cached_nore;

    char *yesexpr, *noexpr;
    int result;

    /* TRANSLATORS: A regular expression testing for an affirmative answer
       (english: "yes"). Testing the first character may be sufficient.
       Take care to consider upper and lower case.
       To enquire the regular expression that your system uses for this
       purpose, you can use the command
            locale -k LC_MESSAGES | grep '^yesexpr=' */
    yesexpr = localized_pattern(N_("^[yY]"), YESEXPR, posixly_correct);
    result = try(response, yesexpr, &last_yesexpr, &cached_yesre);
    if(result < 0)
        return -1;
    if(result)
        return 1;

    /* TRANSLATORS: A regular expression testing for negative answer
       (english: "no"). Testing the first character may be sufficient.
       Take care to consider upper and lower case.
       To enquire the regular expression that your system uses for this
       purpose, you can use the command
            locale -k LC_MESSAGES | grep '^noexpr=' */
    noexpr = localized_pattern(N_("^[nN]"), NOEXPR, posixly_correct);
    result = try(response, noexpr, &last_noexpr, &cached_nore);
    if(result < 0)
        return -1;
    if(result)
        return 0;

    return -1;
