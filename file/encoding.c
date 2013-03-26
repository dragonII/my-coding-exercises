#include "file.h"

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

