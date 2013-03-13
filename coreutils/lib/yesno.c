/* yesno.c -- read a yes/no response from stdin */

#include "yesno.h"

#include <stdlib.h>
#include <stdio.h>

/* Return true if we read an affirmative line from standard input.

   Since this function uses stdin, it is suggested that the caller not
   use STDIN_FILENO directly, and also that the line
   atexit(close_stdin) be added to main() */
bool yesno(void)
{
    bool yes;

    char* response = NULL;
    size_t response_size = 0;
    ssize_t response_len = getline(&response, &response_size, stdin);

    if(response_len <= 0)
        yes = false;
    else
    {
        response[response_len - 1] = '\0';
        yes = (rpmatch(response) > 0);
    }

    free(response);

    return yes;
}
