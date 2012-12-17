/*
 * Routines to open/close/read/write the local file.
 * For "binary" transmission, we use the UNIX open/read/write
 * system calls.
 * For "ascii" transmission, we use the UNIX standard i/o rountines
 * fopen/getc/putc.
 */

#include "defs.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 
 * The following are used by the function in this file only.
 */

static int lastcr   = 0;    // 1 if last character was a carriage-return
static int nextchar = 0;

/*
 * Open the local file for reading or writing.
 * Return a FILE pointer, or NULL on error.
 */

FILE* file_open(char* filename, char* mode, int initblknum)
{
    FILE* fp;

    if(strcmp(filename, "-") == 0)
        fp = stdout;
    else if((fp = fopen(filename, mode)) == NULL)
        return ((FILE*)0);

    nextblknum = initblknum;    // for first data packet or first ACK
    lastcr     = 0;             // for file_write()
    nextchar   = -1;            // for file_read()

    D_printf("file_open: opened %s, mode = %s\n", filename, mode);

    return fp;
}

/*
 * Close the local file.
 * This causes the standard i/o system to flush its buffers for this file
 */

void file_close(FILE* fp)
{
    if(lastcr)
    {
        D_printf("file_close: final character was a CR\n");
        exit(-1);
    }
    if(nextchar >= 0)
    {
        D_printf("file_close: nextchar >= 0\n");
        exit(-1);
    }

    if(fp == stdout)
        return;         // don't close standard output
    else if(fclose(fp) == EOF)
    {
        D_printf("file_close: fclose error\n");
        exit(-1);
    }
}


/*
 * Read data from the local file.
 * Here is where we handle any conversion between the file's mode
 * on the local system and the network mode.
 *
 * Return the number of bytes read (between 1 and maxnbytes, inclusive)
 * or 0 on EOF.
 */
int file_read(FILE* fp, char* ptr, int maxnbytes, int mode)
{
    int c, count;

    if(mode == MODE_BINARY)
    {
        count = read(fileno(fp), ptr, maxnbytes);
        if(count < 0)
        {
            D_printf("file_read: read error on local file.\n");
            exit(-1);
        }
        return count;
    } else if(mode == MODE_ASCII)
    {
        /*
         * For files that are transferred in netascii, we must
         * perform the reverse conversions that file_write() does.
         * Note that we have to use the global "nextchar" to
         * remember if the next character to output is a linefeed
         * or a null, since the second byte of a 2-byte sequence
         * may not fit in the current buffer, and may have to go
         * as the first byte of the next buffer (i.i., we have to
         * remember this fact from one call to the next).
         */

        for(count = 0; count < maxnbytes; count++)
        {
            if(nextchar >= 0)
            {
                *ptr++ = nextchar;
                nextchar = -1;
                continue;
            }

            c = getc(fp);

            if(c == EOF)
            {
                if(ferror(fp))
                {
                    D_printf("file_read: read err from getc on local file.\n");
                    exit(-1);
                }
                return count;
            } else if(c == '\n')
            {
                c = '\r';           // newline -> CR, LF
                nextchar = '\n';
            } else if(c == '\r')
            {
                nextchar = '\0';    // CR -> CR, NULL
            } else
            {
                nextchar = -1;
            }

            *ptr++ = c;
        }
        return count;
    } else
    {
        D_printf("file_read: unknown MODE value\n");
        exit(-1);
    }
}

/*
 * Write data to the local file.
 * Here is where we handle any conversion between the mode of the
 * file on the network and the local system's convertions.
 */
void file_write(FILE* fp, char* ptr, int nbytes, int mode)
{
    int c, i;

    if(mode == MODE_BINARY)
    {
        /* 
         * For binary mode files, no conversion is required.
         */

        i = write(fileno(fp), ptr, nbytes);
        if(i != nbytes)
        {
            D_printf("file_write: write error to local file, i = %d\n", i);
            exit(-1);
        }
    } else if(mode == MODE_ASCII)
    {
        /*
         * For files that are transferred in netascii, we must
         * perform the following conversions:
         *
         *   CR, LF           -> newline  = '\n'
         *   CR, NULL         -> CR       = '\r'
         *   CR, anything_else-> undefined (we don't allow this)
         *
         * Note that we have to use the global "lastcr" to remember
         * if the last character was a carriage-return or not,
         * since if the last character of a buffer is a CR, we have
         * to remember that when we're called for the next buffer.
         */

        for(i = 0; i < nbytes; i++)
        {
            c = *ptr++;
            if(lastcr)
            {
                if(c == '\n')
                    c = '\n';
                else if(c == '\0')
                    c = '\r';
                else
                {
                    D_printf("file_write: CR followed by 0x%02x\n", c);
                    exit(-1);
                }
                lastcr = 0;
            } else if(c == '\r')
            {
                lastcr = 1;
                continue;   // get next character
            }

            if(putc(c, fp) == EOF)
            {
                D_printf("file_write: write error from putc to local file\n");
                exit(-1);
            }
        }
    } else
    {
        D_printf("file_write: unknown MODE value\n");
        exit(-1);
    }
}
