#include "file.h"

/* This table reflects a particular philosophy about what constitutes
   "text," and there is room for disagreement about it.

   Version 3.31 of the file command considered a file to be ASCII if
   each of its character was approved by either the isascii() or
   isalpha() function. On most systems, this would mean that any
   file consisting only of characters in the range 0x00 ... 0x7f
   would be called ASCII text, but many system might reasonably
   consider some characters outside this range to be alphabetic,
   so the file command would call such characters ASCII. It might
   have been more accurate to call this "considered textual on the
   local system" than "ASCII".

   It considered a file to be "International language text" if each
   of its characters was either an ASCII printing character (according
   to the real ASCII standard, not the above test), a character in
   the range 0x80 ... 0xFF, or one of the following control characters:
   backspace, tab, line feed, vertical tab, form feed, carriage return,
   escape. No attempt was made to determine the language in which files
   of this type were written.

   The table below considers a file to be ASCII if all of its characters
   are either ASCII printing characters (again, according to the X3.4
   standard, not isascii()) or any of the following controls: bell,
   backspace, tab, line feed, form feed, carriage return, etc, nextline.

   I include bell because some programs (particularly shell script)
   use it literally, even though it is rare in normal text. I exclude
   vertical tab because it never seems to be used in real text. I alse
   include, with hesitation, the X3.64/ECMA-43 control nextline (0x85),
   because that's what the dd EBCDIC->ASCII table maps the EBCDIC newline
   character to. It might be more appropriate to include it in the 8859
   set instead of the ASCII set, but it's got be be included in "*something*
   we recognize or EBCDIC files aren't going to be considered textual.
   Some old UNIX source files use SO/SI (^N/^O) to shift between Greek
   and Latin characters, so these should possibly be allowed. But they
   make a real mess on VT100-style displays if they're not paired properly,
   so we are probably better off not calling them text.

   A file is considered to be ISO-8859 text if its characters are all
   either ASCII, according to the above difinition, or printing characters
   from the ISO-8859 8-bit extension, characters 0xA0 ... 0xFF.

   Finally, a file is considered to be international text from some other
   character code if its characters are all either ISO-8859 (according to
   the above definition) or characters in the range 0x80 ... 0x9F, which
   ISO-8859 considers to be control characters but the IBM PC and Macintosh
   consider to be printing characters. */

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

static char text_chars[256] = 
{
    /*                  BEL BS HT LF    FF CR    */
    F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
    /*                              ESC          */
    F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
    /*            NEL                            */
    X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
    X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
};



/* Decide whether some text looks UTF-8. Returns:
        -1: invalid UTF-8
         0: uses odd control characters, so doesn't look like text
         1: 7-bit text
         2: definitely UTF-8 text (valid high-bit set bytes)
   If ubuf is non-nul on entry, text is decoded into ubuf, *ulen;
   ubuf must be big enough */
int file_looks_utf8(const unsigned char* buf, size_t nbytes,
                    unichar* ubuf, size_t* ulen)
{
    size_t i;
    int n;
    unichar c;
    int gotone = 0, ctrl = 0;

    if(ubuf)
        *ulen = 0;

    for(i = 0; i < nbytes; i++)
    {
        if((buf[i] & 0x80) == 0)    /* 0xxxxxxx is plain ASCII */
        {
            /* Even if the whole file is valid UTF-8 sequences,
               still reject it if it uses weird control characters */
            if(text_chars[buf[i]] != T)
                ctrl = 1;

            if(ubuf)
                ubuf[(*ulen)++] = buf[i];
        } else if((buf[i] & 0x40) == 0)     /* 10xxxxxx never 1st byte */
            return -1;
        else       /* 11xxxxxx begins UTF-8 */
        {
            int following;

            if((buf[i] & 0x20) == 0)    /* 110xxxxx */
            {
                c = buf[i] & 0x1f;
                following = 1;
            } else if((buf[i] & 0x10) == 0) /* 1110xxxx */
            {
                c = buf[i] & 0x0f;
                following = 2;
            } else if((buf[i] & 0x08) == 0) /* 11110xxx */
            {
                c = buf[i] & 0x07;
                following = 3;
            } else if((buf[i] & 0x04) == 0) /* 111110xx */
            {
                c = buf[i] & 0x03;
                following = 4;
            } else if((buf[i] & 0x02) == 0) /* 1111110x */
            {
                c = buf[i] & 0x01;
                following = 5;
            } else
                return -1;

            for(n = 0; n < following; n++)
            {
                i++;
                if(i >= nbytes)
                    goto done;

                if((buf[i] & 0x80) == 0 || (buf[i] & 0x40))
                    return -1;

                c = (c << 6) + (buf[i] & 0x3f);
            }

            if(ubuf)
                ubuf[(*ulen)++] = c;
            gotone = 1;
        }
    }
done:
    return ctrl ? 0 : (gotone ? 2 : 1);
}

