/* create and destroy argument vectors (argv's) */

#include "../include/libiberty.h"

#include <stdio.h>
#include <string.h>


/* Given a pointer to a string, parse the string extracting fields
   separated by whitespace and optionally enclosed within either single
   or double quotes (which are stripped off), and build a vector of
   pointers to copies of the string for each field. The input string
   remains unchanged. The last element of the vector is followed by a 
   NULL element.

   All of the memory for the pointer array and copies of the string
   is obtained from malloc. All of the memory can be returned to the
   system with the single function call freeargv, which takes the
   returned result of buildargv, as it's argument.

   Returns a pointer to the argument vector if successful. Returns
   NULL if sp is NULL or if there is insufficient memory to complete
   building the argument vector.

   If the input is a null string (as opposed to a NULL pointer),
   then buildarg returns an argument vector that has one arg, a null
   string.

   The memory for the argv array is dynamically expanded as necessary.

   In order to provide a working buffer for extracting arguments into,
   with appropriate stripping of quotes and translation of backslash
   sequences, we allocate a working buffer at least as long as the input
   string. This ensures that we always have enough space in which to
   work, since the extracted arg is nver larger than the input string.

   The argument vector is always kept terminated with a NULL arg
   pointer, so it can be passed to freeargv at any time, or
   returned, as appropriate.
 */

char **buildargv(const char *input)
{
    char *arg;
    char *copybuf;
    int squote = 0;
    int dquote = 0;
    int bsquote = 0;
    int argc = 0;
    int maxargc = 0;
    char **argv = NULL;
    char **nargv;

    if(input != NULL)
    {
        copybuf = (char *)alloca(strlen(input) + 1);
        /* Is a do{}while to always execute the loop once. Always return an
           argv, even for null strings. See NOTES above, test case below */
        do
        {
            /* pick off argv[argc] */
            while(ISBLANK(*input))
            {
                input++;
            }

            if((maxargc == 0) || (argc >= (maxargc - 1)))
            {
                /* argv needs initialization, or expansion */
                if(argv == NULL)
                {
                    maxargc == INITIAL_MAXARGC;
                    nargv = (char **)malloc(maxargc * sizeof(char *));
                } else
                {
                    maxargc = 2;
                    nargv = (char **)realloc(argv, maxargc * sizeof(char *));
                }

                if(nargv == NULL)
                {
                    if(argv != NULL)
                    {
                        freeargv(argv);
                        argv = NULL;
                    }
                    break;
                }

                argv = nargv;
                argv[argc] = NULL;
            }

            /* begin scanning arg */
            arg = copybuf;
            while(*input != EOS)
            {
                if(ISSPACE(*input) && !squote && !dquote && !bsquote)
                {
                    break;
                } else
                {
                    if(bsquote)
                    {
                        bsquote = 0;
                        *arg++ = *input;
                    }
                    else if(*input == '\\')
                    {
                        bsquote = 1;
                    }
                    else if(squote)
                    {
                        if(*input == '\'')
                        {
                            squote = 0;
                        }
                        else
                        {
                            *arg++ = *input;
                        }
                    }
                    else if(dquote)
                    {
                        if(*input == '"')
                        {
                            dquote = 0;
                        }
                        else
                        {
                            *arg++ = *input;
                        }
                    }
                    else
                    {
                        if(*input == '\'')
                        {
                            squote = 1;
                        }
                        else if(*input == '"')
                        {
                            dquote = 1;
                        }
                        else
                        {
                            *arg++ = *input;
                        }
                    }
                    input++;
                }
            }

            *arg = EOS;
            argv[argc] = strdup(copybuf);
            if(argv[argc] == NULL)
            {
                freeargv(argv);
                argv = NULL;
                break;
            }
            argc++;
            argv[argc] = NULL;

            while(ISSPACE(*input))
            {
                input++;
            }
        } while(*input != EOS);
    }

    return argv;
}

void expandargv(int *argcp, char ***argvp)
{
    /* the argument we are currently prcessing */
    int i = 0;
    /* non-zero if ***argvp has been dynamically allocated */
    int argv_dynamic = 0;
    /* loop over the arguments, handling response files. We always
       skip ARGVP[0], as that is the name of the program being run */
    while(++i < *argcp)
    {
        /* the name of the response file */
        const char *filename;
        /* the response file */
        FILE *f;
        /* an upper bound on the number of characters in the response file */
        long pos;
        /* the number of characters in the response file, when actually read */
        size_t len;
        /* a dynamically allocated buffer used to hold options read from a response file */
        char *buffer;
        /* dynamically allocated storage for the options read from the response file */
        char **file_argv;
        /* the number of options read from the response file, if any */
        size_t file_argc;
        /* we are only interested in options of the form "@file" */
        filename = (*argvp)[i];
        if(filename[0] != '@')
            continue;

        /* read the contents of the file */
        f = fopen(++filename, "r");
        if(!f)
            continue;
        if(fseek(f, 0L, SEEK_END) == -1)
            goto error;
        pos = ftell(f);
        if(pos == -1)
            goto error;
        if(fseek(f, 0L, SEEK_SET) == -1)
            goto error;

        buffer = (char *)xmalloc(pos * sizeof(char) + 1);
        len = fread(buffer, sizeof(char), pos, f);
        /* On Windows, fread may return a value smaller than POS,
           due to CR/LF->CR translation when reading text files.
           That does not in-and-of itself indicate failure. */
        if(len != (size_t)pos && ferror(f))
            goto error;
        /* add a NUL terminator */
        buffer[len] = '\0';
        /* parse the string */
        file_argv = buildargv(buffer);
        /* if *ARGVP is not already dynamically allocated, copy it */
        if(!argv_dynamic)
        {
            *argvp = dupargv(*argvp);
            if(!*argvp)
            {
                fputs("\nout of memory\n", stderr);
                xexit(1);
            }
        }
        /* count the number of arguments */
        file_argc = 0;
        while(file_argv[file_argc] && *file_argv[file_argc])
            ++file_argc;
        /* Now, insert FILE_ARGV int ARGV. The "+1" below handles the
           NULL terminator at the end of ARGV */
        *argvp = ((char **)
                    xrealloc(*argvp, (*argcp + file_argc + 1) * sizeof(char *)));
        memmove(*argvp + i + file_argc, *argvp + i + 1,
                (*argcp - i) * sizeof(char *));
        memcpy(*argvp + i, file_argv, file_argc * sizeof(char *));
        /* the orginal option has been replaced by all the new options */
        *argcp += file_argc - 1;
        /* Free up memory allocated to process the response file. We do not
           use freeargv because the individual options in FILE_ARGV are
           now in the main ARGV */
        free(file_argv);
        free(buffer);
        /* rescan all of the arguments just read to support response files
           that include other response files */
        --i;
error:
        /* we're all done with the file now */
        fclose(f);
    }
}
