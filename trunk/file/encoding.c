/* encoding -- determine the character encoding of a text file */

#include "file_.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG_ENCODING
# define DPRINTF(a) printf a
#else
# define DPRINTF(a)
#endif

static int looks_ascii(const unsigned char*, size_t, unichar*, size_t*);
static int looks_utf8_with_BOM(const unsigned char*, size_t, unichar*, size_t*);
static int looks_ucs16(const unsigned char*, size_t, unichar*, size_t*);
static int looks_latin1(const unsigned char*, size_t, unichar*, size_t*);
static int looks_extended(const unsigned char*, size_t, unichar*, size_t*);
static void from_ebcdic(const unsigned char*, size_t, unsigned char*);


/* try to determine whether text is in some character code we can
   identify. Each of these tests, if it succeeds, will leave
   the text converted into one-unichar-per-character Unicode in
   ubuf, and the number of characters converted in ulen */
int file_encoding(struct magic_set* ms, const unsigned char* buf, 
                  size_t nbytes, unichar** ubuf, size_t* ulen, 
                  const char** code, const char** code_mime,
                  const char** type)
{
    size_t mlen;
    int rv = 1, ucs_type;
    unsigned char* nbuf = NULL;

    *type = "text";
    mlen = (nbytes + 1) * sizeof(nbuf[0]);
    if((nbuf = CAST(unsigned char*, calloc((size_t)1, mlen))) == NULL)
    {
        file_oomem(ms, mlen);
        goto done;
    }
    mlen = (nbytes + 1) * sizeof((*ubuf)[0]);
    if((*ubuf = CAST(unichar*, calloc((size_t)1, mlen))) == NULL)
    {
        file_oomem(ms, mlen);
        goto done;
    }

    if(looks_ascii(buf, nbytes, *ubuf, ulen))
    {
        DPRINTF(("ascii %" SIZE_T_FORMAT "u\n", *ulen));
        *code = "ASCII";
        *code_mime = "us-ascii";
    } else if(looks_utf8_with_BOM(buf, nbytes, *ubuf, ulen) > 0)
    {
        DPRINTF(("utf8/bom %" SIZE_T_FORMAT "u\n", *ulen));
        *code = "UTF-8 Unicode (with BOM)";
        *code_mime = "utf-8";
    } else if(file_looks_utf8(buf, nbytes, *ubuf, ulen) > 1)
    {
        DPRINTF(("utf8 %" SIZE_T_FORMAT "u\n", *ulen));
        *code = "UTF-8 Unicode (with BOM)";
        *code = "UTF-8 Unicode";
        *code_mime = "utf-8";
    } else if((ucs_type = looks_ucs16(buf, nbytes, *ubuf, ulen)) != 0)
    {
        if(ucs_type == 1)
        {
            *code = "Little-endian UTF-16 Unicode";
            *code_mime = "utf-16le";
        } else
        {
            *code = "Big-endian UTF-16 Unicode";
            *code_mime = "utf-16be";
        }
        DPRINTF(("ucs16 %" SIZE_T_FORMAT "u\n", *ulen));
    } else if(looks_latin1(buf, nbytes, *ubuf, ulen))
    {
        DPRINTF(("latin1 %" SIZE_T_FORMAT "u\n", *ulen));
        *code = "ISO-8859";
        *code_mime = "iso-8859-1";
    } else if(looks_extended(buf, nbytes, *ubuf, ulen))
    {
        DPRINTF(("extended %" SIZE_T_FORMAT "u\n", *ulen));
        *code = "Non-ISO extended-ASCII";
        *code_mime = "unknown-8bit";
    } else
    {
        from_ebcdic(buf, nbytes, nbuf);

        if(looks_ascii(nbuf, nbytes, *ubuf, ulen))
        {
            DPRINTF(("ebcdic %" SIZE_T_FORMAT "u\n", *ulen));
            *code = "EBCDIC";
            *code_mime = "ebcdic";
        } else if(looks_latin1(nbuf, nbytes, *ubuf, ulen))
        {
            DPRINTF(("ebcdic/international %" SIZE_T_FORMAT "u\n", *ulen));
            *code = "International EBCDIC";
            *code_mime = "ebcdic";
        } else
        {
            /* Doesn't look like text at all */
            DPRINTF(("binary"));
            rv = 0;
            *type = "binary";
        }
    }
done:
    free(nbuf);

    return rv;
}


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


static int looks_ascii(const unsigned char* buf, size_t nbytes,
                       unichar* ubuf, size_t* ulen)
{
    size_t i;

    *ulen = 0;

    for(i = 0; i < nbytes; i++)
    {
        int t = text_chars[buf[i]];

        if(t != T)
            return 0;

        ubuf[(*ulen)++] = buf[i];
    }

    return 1;
}


/* Decide whether some text looks like UTF-8 with BOM. If there is no
   BOM, return -1; otherwise return the result of looks_utf8 on the
   rest of the text */
static int looks_utf8_with_BOM(const unsigned char* buf, size_t nbytes,
                               unichar* ubuf, size_t* ulen)
{
    if(nbytes > 3 && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf)
        return file_looks_utf8(buf + 3, nbytes - 3, ubuf, ulen);
    else
        return -1;
}


static int looks_ucs16(const unsigned char* buf, size_t nbytes,
                       unichar* ubuf, size_t* ulen)
{
    int bigend;
    size_t i;

    if(nbytes < 2)
        return 0;

    if(buf[0] == 0xff && buf[1] == 0xfe)
        bigend = 0;
    else if(buf[0] == 0xfe && buf[1] == 0xff)
        bigend = 1;
    else
        return 0;

    *ulen = 0;

    for(i = 2; i + 1 < nbytes; i += 2)
    {
        /* XXX: fix to properly handle chars > 65536 */

        if(bigend)
            ubuf[(*ulen)++] = buf[i + 1] + 256 * buf[i];
        else
            ubuf[(*ulen)++] = buf[i] + 256 * buf[i + 1];

        if(ubuf[*ulen - 1] == 0xfffe)
            return 0;
        if(ubuf[*ulen - 1] < 128 &&
                text_chars[(size_t)ubuf[*ulen - 1]] != T)
            return 0;
    }

    return 1 + bigend;
}


static int looks_latin1(const unsigned char* buf, size_t nbytes,
                        unichar* ubuf, size_t* ulen)
{
    size_t i;
    *ulen = 0;

    for(i = 0; i < nbytes; i++)
    {
        int t = text_chars[buf[i]];

        if(t != T && t != I)
            return 0;

        ubuf[(*ulen)++] = buf[i];
    }

    return 1;
}


static int looks_extended(const unsigned char* buf, size_t nbytes,
                          unichar* ubuf, size_t* ulen)
{
    size_t i;
    *ulen = 0;

    for(i = 0; i < nbytes; i++)
    {
        int t = text_chars[buf[i]];

        if(t != T && t != I && t != X)
            return 0;

        ubuf[(*ulen)++] = buf[i];
    }

    return 1;
}


#undef F
#undef T
#undef I
#undef X


/* This table maps each EBCDIC character to an (8-bit extended) ASCII
   character, as specified in the rationale for the dd(1) command in
   draft 11.2 (September, 1991) of the POSIX P1003.2 standard.

   Unfortunately it doen not seem to correspond exactly to any of the 
   five variants of EBCDIC documented in IBM's _Enterprise Systems
   Architecture/390: Principles of Operation_, SA22-7201-06, Seventh
   Edition, July, 1999, pp. I-1 - I-4.

   Fortunately, though, all versions of EBCDIC, including this one, agree
   on most of the printing characters that also appears in (7-bit) ASCII.
   Of these, only '|', '!', '~', '^', '[' and ']' are in question at all.

   Fortunately too, there is general agreement that codes 0x00 through
   0x3F represent control characters, 0x41 a nonbreaking space, and the
   remainder printing characters.

   This is sufficient to allow us to identify EBCDIC text and to distinguish
   between old-style and internationalized examples of text. */
static unsigned char ebcdic_to_ascii[] = {
  0,   1,   2,   3, 156,   9, 134, 127, 151, 141, 142,  11,  12,  13,  14,  15,
 16,  17,  18,  19, 157, 133,   8, 135,  24,  25, 146, 143,  28,  29,  30,  31,
128, 129, 130, 131, 132,  10,  23,  27, 136, 137, 138, 139, 140,   5,   6,   7,
144, 145,  22, 147, 148, 149, 150,   4, 152, 153, 154, 155,  20,  21, 158,  26,
' ', 160, 161, 162, 163, 164, 165, 166, 167, 168, 213, '.', '<', '(', '+', '|',
'&', 169, 170, 171, 172, 173, 174, 175, 176, 177, '!', '$', '*', ')', ';', '~',
'-', '/', 178, 179, 180, 181, 182, 183, 184, 185, 203, ',', '%', '_', '>', '?',
186, 187, 188, 189, 190, 191, 192, 193, 194, '`', ':', '#', '@', '\'','=', '"',
195, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 196, 197, 198, 199, 200, 201,
202, 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', '^', 204, 205, 206, 207, 208,
209, 229, 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 210, 211, 212, '[', 214, 215,
216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, ']', 230, 231,
'{', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 232, 233, 234, 235, 236, 237,
'}', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 238, 239, 240, 241, 242, 243,
'\\',159, 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 244, 245, 246, 247, 248, 249,
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 250, 251, 252, 253, 254, 255
};

/* Copy buf[0 ... nbytes-1] into out[], translating EBCDIC to ASCII */
static void from_ebcdic(const unsigned char* buf, size_t nbytes, unsigned char* out)
{
    size_t i;

    for(i = 0; i < nbytes; i++)
        out[i] = ebcdic_to_ascii[buf[i]];
}
