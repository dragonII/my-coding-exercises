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

/* The traditional octal values corresponding to each mode bit. */
#define SUID 04000
#define SGID 02000
#define SVTX 01000
#define RUSR 00400
#define WUSR 00200
#define XUSR 00100
#define RGRP 00040
#define WGRP 00020
#define XGRP 00010
#define ROTH 00004
#define WOTH 00002
#define XOTH 00001
#define ALLM 07777  /* all octal mode bits */

/* Convert OCTAL, which uses one of the traditional octal values, to
   an internal mode_t value. */
static mode_t octal_to_mode(unsigned int octal)
{
    /* Help the compiler optimize the usal case whrer mode_t uses
       the traditional octal representation */

    return ((S_ISUID == SUID && S_ISGID == SGID && S_ISVTX == SVTX
          && S_IRUSR == RUSR && S_IWUSR == WUSR && S_IXUSR == XUSR
          && S_IRGRP == RGRP && S_IWGRP == WGRP && S_IXGRP == XGRP
          && S_IROTH == ROTH && S_IWOTH == WOTH && S_IXOTH == XOTH)
            ? octal
            : (mode_t)   ((octal & SUID ? S_ISUID : 0)
                        | (octal & SGID ? S_ISGID : 0)
                        | (octal & SVTX ? S_ISVTX : 0)
                        | (octal & RUSR ? S_IRUSR : 0)
                        | (octal & WUSR ? S_IWUSR : 0)
                        | (octal & XUSR ? S_IXUSR : 0)
                        | (octal & RGRP ? S_IRGRP : 0)
                        | (octal & WGRP ? S_IWGRP : 0)
                        | (octal & XGRP ? S_IXGRP : 0)
                        | (octal & ROTH ? S_IROTH : 0)
                        | (octal & WOTH ? S_IWOTH : 0)
                        | (octal & XOTH ? S_IXOTH : 0)));
}

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

        /* Convert from octal to decimal */
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
