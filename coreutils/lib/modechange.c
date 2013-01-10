/* modechange.c -- file mode manipulation 

   The ASCII mode string is compilied into an array of `struct
   modechange', which can ghen be applied to each file to be changed.
   We do this instead of re-parsing the ASCII string for each file
   because the compiled form requires less computation to use; when
   changing the mode of many files, this probably results in a 
   performance gain */

#include <sys/stat.h>
#include <stdlib.h>

#include "modechange.h"
#include "xalloc.h"

/* Descriptition of a mode change */
struct mode_change
{
    char op;                /* One of "=+-" */
    char flag;              /* Special operations flag */
    mode_t affected;        /* Set for u, g, o, or a */
    mode_t value;           /* Bits to add/remove */
    mode_t mentioned;       /* Bit explicitly mentioned */
};

/* Return a pointer to an array of file mode change operations created from
   MODE_STRING, an ASCII string that contains either an octal number
   specifying an absolute mode, or symbolic mode change operations with 
   the form:
   [ugoa...][[+-=][rwxXstugo...]...][,...]

   Return NULL if `mode_string' does contain a valid
   representation of file mode change operations. */
struct mode_change* mode_compile(char* mode_string)
{
    /* The array of mode-change directives to be returned */
    struct mode_change* mc;
    size_t used = 0;

    if(*mode_string >= '0' && *mode_string < '8')
    {
        unsigned int octal_mode = 0;
        mode_t mode;
        mode_t mentioned;

        do
        {
            octal_mode = 8 * octal_mode + *mode_string++ - '0';
            if(ALLM < octal_mode)
                return NULL;
        }
        while(*mode_string >= '0' && *mode_string < '8');

        if(*mode_string)
            return NULL;

        mode = octal_to_mode(octal_mode);
        mentioned = (mode & (S_ISUID | S_ISGID)) | S_ISVTX | S_IRWXUGO;
        return make_node_op_equals(mode, mentioned);
    }

    /* Allocate enough space to hold the result */
    {
        size_t needed = 1;
        char* p;
        for(p = mode_string; *p; p++)
            needed += (*p == '=' || *p == '+' || *p == '-');
        mc = xnmalloc(needed, sizeof *mc);
    }

    /* One loop iteration for each `[ugoa]*([-+=]([rwxXst]*|[ugo]))+' */
    for(;; mode_string++)
    {
        /* which bits in the mode are operated on */
        mode_t affected = 0;

        /* Turn on all the bits in `affected' for each group given */
        for(;; mode_string++)
            switch(*mode_string)
            {
                default:
                    goto invalid;
                case 'u':
                    affected |= S_ISUID | S_IRWXU;
                    break;
                case 'g':
                    affected |= S_ISGID | S_IRWXG;
                    break;
                case 'o':
                    affected |= S_ISVTX | S_IRWXO;
                    break;
                case 'a':
                    affected |= CHMOD_MODE_BITS;
                    break;
                case '=':
                case '+':
                case '-':
                    goto no_more_affected;
            }
        no_more_affected:;

        do
        {
            char op = *mode_string;
            mode_t value;
            char flag = MODE_COPY_EXISTING;
            struct mode_change* change;

            switch(*mode_string)
            {
                case 'u':
                    /* Set the affected bits to the value of the `u' bits
                       on the same file */
                    value = S_IRWXU;
                    break;
                case 'g':
                    /* Set the affected bits to the value of the `g' bits
                       on the same file */
                   value = S_IRWXG;
                   break;
                case 'o':
                    /* Set the affected bits to the value of the `o' bits
                       on the same file */
                    value = S_IRWXO;
                    break;

                default:
                    value = 0;
                    flag = MODE_ORDINARY_CHANGE;

                for(mode_string--; ; mode_string++)
                    switch(*mode_string)
                    {
                        case 'r':
                            value |= S_IRUSR | S_IRGRP | S_IROTH;
                            break;
                        case 'w':
                            value |= S_IWUSR | S_IWGRP | S_IWOTH;
                            break;
                        case 'x':
                            value |= S_IXUSR | S_IXGRP | S_IXOTH;
                            break;
                        case 'X':
                            flag = MODE_X_IF_ANY_X;
                            break;
                        case 's':
                            /* Set the setuid/gid bits if `u' or `g' is selected */
                            value |= S_ISUID | S_ISGID;
                            break;
                        case 't':
                            /* Set the "save text image" bit if `o' is selected */
                            value |= S_ISVTX;
                            break;
                        default:
                            goto no_more_values;
                    }
                no_more_values: ;
            }

            change = &mc[used++];
            change->op = op;
            change->flag = flag;
            change->affected = affected;
            change->value = value;
            change->mentioned = (affected ? affected & value : value);
        }
        while(*mode_string == '=' || *mode_string == '+'
                || *mode_string == '-');

        if(*mode_string != ',')
            break;
    }
    if(*mode_string == 0)
    {
        mc[used].flag = MODE_DONE;
        return mc;
    }
invalid:
    free(mc);
    return NULL;
}
