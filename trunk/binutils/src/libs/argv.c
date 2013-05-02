/* create and destroy argument vectors (argv's) */

#include "../include/libiberty.h"

#include <stdio.h>
#include <string.h>

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
