/* softmagic - interpret variable magic from MAGIC */

#include "file_.h"
#include "magic_.h"

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>


static int mget(struct magic_set*, const unsigned char*,
                struct magic*, size_t, size_t, unsigned int, int, int, int, int*);
static int mcopy(struct magic_set*, union VALUETYPE*, int, int,
                const unsigned char*, uint32_t, size_t, size_t);
static int match(struct magic_set*, struct magic*, uint32_t,
                const unsigned char*, size_t, size_t, int, int, int, int*);

static int
mcopy(struct magic_set* ms, union VALUETYPE* p, int type, int indir,
        const unsigned char* s, uint32_t offset, size_t nbytes, size_t linecnt)
{
    /* Note: FILE_SEARCH and FILE_REGEX do not actually copy
       anything, but setup pointers into the source */
    if(indir == 0)
    {
        switch(type)
        {
            case FILE_SEARCH:
                ms->search.s = RCAST(const char*, s) + offset;
                ms->search.s_len = nbytes - offset;
                ms->search.offset = offset;
                return 0;
            case FILE_REGEX:
            {
                const char *b, *c;
                const char* last;   /* end of search region */
                const char* buf;    /* start of search region */
                const char* end;
                size_t lines;

                if(s == NULL)
                {
                    ms->search.s_len = 0;
                    ms->search.s = NULL;
                    return 0;
                }
                buf = RCAST(const char*, s) + offset;
                end = last = RCAST(const char*, s) + nbytes;
                /* mget() guarantees buf <= last */
                for(lines = linecnt, b = buf; lines && b < end  &&
                    ((b = CAST(const char*, memchr(c = b, '\n', CAST(size_t, (end - b)))))
                    || (b = CAST(const char*, memchr(c, '\r', CAST(size_t, (end - c))))));
                    lines--, b++)
                {
                    last = b;
                    if(b[0] == '\r' && b[1] == '\n')
                        b++;
                }
                if(lines)
                    last = RCAST(const char*, s) + nbytes;

                ms->search.s = buf;
                ms->search.s_len = last - buf;
                ms->search.offset = offset;
                ms->search.rm_len = 0;
                return 0;
            }
            case FILE_BESTRING16:
            case FILE_LESTRING16:
            {
                const unsigned char* src = s + offset;
                const unsigned char* esrc = s + nbytes;
                char* dst = p->s;
                char* edst = &p->s[sizeof(p->s) - 1];

                if(type == FILE_BESTRING16)
                    src++;

                /* Check that offset is within range */
                if(offset >= nbytes)
                {
                    file_magerror(ms, "invalid offset %u in mcopy()",
                                    offset);
                    return -1;
                }
                for(; src < esrc; src += 2, dst++)
                {
                    if(dst < edst)
                        *dst = *src;
                    else
                        break;
                    if(*dst == '\0')
                    {
                        if(type == FILE_BESTRING16 ?
                            *(src - 1) != '\0' :
                            *(src + 1) != '\0')
                            *dst = ' ';
                    }
                }
                *edst = '\0';
                return 0;
            }
            case FILE_STRING:   /* these two should not need */
            case FILE_PSTRING:  /* to copy anything, but do anyway */
            default:
                break;
        }
    }

    if(offset >= nbytes)
    {
        memset(p, '\0', sizeof(*p));
        return 0;
    }

    if(nbytes - offset < sizeof(*p))
        nbytes = nbytes - offset;
    else
        nbytes = sizeof(*p);

    memcpy(p, s + offset, nbytes);

    /* the usefulness of padding with zeros eludes me, it
       might even cause problems */
    if(nbytes < sizeof(*p))
        memset(((char*)(void*)p) + nbytes, '\0',
            sizeof(*p) - nbytes);
    return 0;
}


static void
mdebug(uint32_t offset, const char* str, size_t len)
{
    fprintf(stderr, "mget/%zu @%d: ", len, offset);
    file_showstr(stderr, str, len);
    fputc('\n', stderr);
    fputc('\n', stderr);
}


static int
cvt_flip(int type, int flip)
{
    if(flip == 0)
        return type;
    switch(type)
    {
        case FILE_BESHORT:
            return FILE_LESHORT;
        case FILE_BELONG:
            return FILE_LELONG;
        case FILE_BEDATE:
            return FILE_LEDATE;
        case FILE_BELDATE:
            return FILE_LELDATE;
        case FILE_BEQUAD:
            return FILE_LEQUAD;
        case FILE_BEQDATE:
            return FILE_LEQDATE;
        case FILE_BEQLDATE:
            return FILE_LEQLDATE;
        case FILE_BEQWDATE:
            return FILE_LEQWDATE;
        case FILE_LESHORT:
            return FILE_BESHORT;
        case FILE_LELONG:
            return FILE_BELONG;
        case FILE_LEDATE:
            return FILE_BEDATE;
        case FILE_LELDATE:
            return FILE_BELDATE;
        case FILE_LEQUAD:
            return FILE_BEQUAD;
        case FILE_LEQDATE:
            return FILE_BEQDATE;
        case FILE_LEQLDATE:
            return FILE_BEQLDATE;
        case FILE_LEQWDATE:
            return FILE_BEQWDATE;
        case FILE_BEFLOAT:
            return FILE_LEFLOAT;
        case FILE_LEFLOAT:
            return FILE_BEFLOAT;
        case FILE_BEDOUBLE:
            return FILE_LEDOUBLE;
        case FILE_LEDOUBLE:
            return FILE_BEDOUBLE;
        default:
            return type;
    }
}


#define DO_CVT(fld, cast)   \
    if(m->num_mask) \
        switch(m->mask_op & FILE_OPS_MASK)  \
        {   \
            case FILE_OPAND:    \
                p->fld &= cast m->num_mask; \
                break;          \
            case FILE_OPOR:     \
                p->fld |= cast m->num_mask; \
                break;          \
            case FILE_OPXOR:    \
                p->fld ^= cast m->num_mask;  \
                break;          \
            case FILE_OPADD:    \
                p->fld += cast m->num_mask; \
                break;          \
            case FILE_OPMULTIPLY:    \
                p->fld *= cast m->num_mask; \
                break;          \
            case FILE_OPMINUS:    \
                p->fld -= cast m->num_mask; \
                break;          \
            case FILE_OPDIVIDE:    \
                p->fld /= cast m->num_mask; \
                break;          \
            case FILE_OPMODULO:    \
                p->fld %= cast m->num_mask; \
                break;          \
        }   \
    if(m->mask_op & FILE_OPINVERSE) \
        p->fld = ~p->fld;   \



static void
cvt_8(union VALUETYPE* p, const struct magic* m)
{
    DO_CVT(b, (uint8_t));
}

static void
cvt_16(union VALUETYPE* p, const struct magic* m)
{
    DO_CVT(h, (uint16_t));
}

static void
cvt_32(union VALUETYPE* p, const struct magic* m)
{
    DO_CVT(l, (uint32_t));
}

static void
cvt_64(union VALUETYPE* p, const struct magic* m)
{
    DO_CVT(q, (uint64_t));
}


#define DO_CVT2(fld, cast)  \
    if(m->num_mask)         \
        switch(m->mask_op & FILE_OPS_MASK)  \
        {   \
            case FILE_OPADD:    \
                p->fld += cast m->num_mask; \
                break;  \
            case FILE_OPMINUS:    \
                p->fld -= cast m->num_mask; \
                break;  \
            case FILE_OPMULTIPLY:    \
                p->fld *= cast m->num_mask; \
                break;  \
            case FILE_OPDIVIDE:    \
                p->fld /= cast m->num_mask; \
                break;  \
        }



static void
cvt_float(union VALUETYPE* p, const struct magic* m)
{
    DO_CVT2(f, (float));
}

static void
cvt_double(union VALUETYPE* p, const struct magic* m)
{
    DO_CVT2(d, (double));
}

/* Convert the byte order of the data we are looking at
   while we're here, let's apply the mask operation */
static int 
mconvert(struct magic_set* ms, struct magic* m, int flip)
{
    union VALUETYPE* p = &ms->ms_value;

    switch(cvt_flip(m->type, flip))
    {
        case FILE_BYTE:
            cvt_8(p, m);
            return 1;
        case FILE_SHORT:
            cvt_16(p, m);
        case FILE_LONG:
        case FILE_DATE:
        case FILE_LDATE:
            cvt_32(p, m);
            return 1;
        case FILE_QUAD:
        case FILE_QDATE:
        case FILE_QLDATE:
        case FILE_QWDATE:
            cvt_64(p, m);
            return 1;
        case FILE_STRING:
        case FILE_BESTRING16:
        case FILE_LESTRING16:
        {
            /* null terminate and eat *trailing* return */
            p->s[sizeof(p->s) - 1] = '\0';
            return 1;
        }
        case FILE_PSTRING:
        {
            char *ptr1 = p->s, *ptr2 = ptr1 + file_pstring_length_size(m);
            size_t len = file_pstring_get_length(m, ptr1);
            if(len >= sizeof(p->s))
                len = sizeof(p->s) - 1;
            while(len--)
                *ptr1++ = *ptr2++;
            *ptr1 = '\0';
            return 1;
        }
        case FILE_BESHORT:
            p->h = (short)((p->hs[0] << 8) | (p->hs[1]));
            cvt_16(p, m);
            return 1;
        case FILE_BELONG:
        case FILE_BEDATE:
        case FILE_BELDATE:
            p->l = (int32_t)
                    ((p->hl[0] << 24) | (p->hl[1] << 16) | (p->hl[2] << 8) | (p->hl[3]));
            cvt_32(p, m);
            return 1;
        case FILE_BEQUAD:
        case FILE_BEQDATE:
        case FILE_BEQLDATE:
        case FILE_BEQWDATE:
            p->q = (uint64_t)
                (((uint64_t)p->hq[0] << 56) | ((uint64_t)p->hq[1] << 48) |
                 ((uint64_t)p->hq[2] << 40) | ((uint64_t)p->hq[3] << 32) |
                 ((uint64_t)p->hq[4] << 24) | ((uint64_t)p->hq[5] << 16) |
                 ((uint64_t)p->hq[6] <<  8) | ((uint64_t)p->hq[7]));
            cvt_64(p, m);
            return 1;
        case FILE_LESHORT:
            p->l = (int32_t)
                ((p->hl[3] << 24) | (p->hl[2] << 16) | (p->hl[1] << 8) | (p->hl[0]));
            cvt_32(p, m);
            return 1;
        case FILE_LEQUAD:
        case FILE_LEQDATE:
        case FILE_LEQLDATE:
        case FILE_LEQWDATE:
            p->q = (uint64_t)
                (((uint64_t)p->hq[7] << 56) | ((uint64_t)p->hq[6] << 48) |
                 ((uint64_t)p->hq[5] << 40) | ((uint64_t)p->hq[4] << 32) |
                 ((uint64_t)p->hq[3] << 24) | ((uint64_t)p->hq[2] << 16) |
                 ((uint64_t)p->hq[1] <<  8) | ((uint64_t)p->hq[0])); 
            cvt_64(p, m);
            return 1;
        case FILE_MELONG:
        case FILE_MEDATE:
        case FILE_MELDATE:
            p->l = (int32_t)
                ((p->hl[1] << 24) | (p->hl[0] << 16) | (p->hl[3] << 8) | (p->hl[2]));
            cvt_32(p, m);
            return 1;
        case FILE_FLOAT:
            cvt_float(p, m);
            return 1;
        case FILE_BEFLOAT:
            p->l = ((uint32_t)p->hl[0] << 24) | ((uint32_t)p->hl[1] << 16) |
                   ((uint32_t)p->hl[2] << 8) | ((uint32_t)p->hl[3]);
            cvt_float(p, m);
            return 1;
        case FILE_LEFLOAT:
            p->l = ((uint32_t)p->hl[3] << 24) | ((uint32_t)p->hl[2] << 16) |
                    ((uint32_t)p->hl[1] << 8) | ((uint32_t)p->hl[0]);
            cvt_float(p, m);
            return 1;
        case FILE_DOUBLE:
            cvt_double(p, m);
            return 1;
        case FILE_BEDOUBLE:
            p->q = ((uint64_t)p->hq[0] << 56) | ((uint64_t)p->hq[1] << 48) |
                   ((uint64_t)p->hq[2] << 40) | ((uint64_t)p->hq[3] << 32) |
                   ((uint64_t)p->hq[4] << 24) | ((uint64_t)p->hq[5] << 16) |
                   ((uint64_t)p->hq[6] <<  8) | ((uint64_t)p->hq[7]); 
            cvt_double(p, m);
            return 1;
        case FILE_LEDOUBLE:
            p->q = ((uint64_t)p->hq[7] << 56) | ((uint64_t)p->hq[6] << 48) |
                   ((uint64_t)p->hq[5] << 40) | ((uint64_t)p->hq[4] << 32) |
                   ((uint64_t)p->hq[3] << 24) | ((uint64_t)p->hq[2] << 16) |
                   ((uint64_t)p->hq[1] <<  8) | ((uint64_t)p->hq[0]); 
            cvt_double(p, m);
            return 1;
        case FILE_REGEX:
        case FILE_SEARCH:
        case FILE_DEFAULT:
        case FILE_NAME:
        case FILE_USE:
            return 1;
        default:
            file_magerror(ms, "invalid type %d in mconvert()", m->type);
            return 0;
    }
}



static int
mget(struct magic_set* ms, const unsigned char* s, struct magic* m,
    size_t nbytes, size_t o, unsigned int cont_level, int mode, int text,
    int flip, int* returnval)
{
    uint32_t offset = ms->offset;
    uint32_t count = m->str_range;
    int rv;
    char *sbuf, *rbuf;
    union VALUETYPE* p = &ms->ms_value;
    struct mlist ml;

    if(mcopy(ms, p, m->type, m->flag & INDIR, s, (uint32_t)(offset + o),
                (uint32_t)nbytes, count) == -1)
        return -1;

    if((ms->flags & MAGIC_DEBUG) != 0)
    {
        fprintf(stderr, "mget(type=%d, flag=%x, offset=%u, o=%zu, "
                        "nbytes=%zu, count=%u)\n",
                m->type, m->flag, offset, o, nbytes, count);
        mdebug(offset, (char*)(void*)p, sizeof(union VALUETYPE));
        file_mdump(m);
    }

    if(m->flag & INDIR)
    {
        int off = m->in_offset;
        if(m->in_op & FILE_OPINDIRECT)
        {
            const union VALUETYPE* q = CAST(const union VALUETYPE*,
                    ((const void*)(s + offset + off)));
            switch(cvt_flip(m->in_type, flip))
            {
                case FILE_BYTE:
                    off = q->b;
                    break;
                case FILE_SHORT:
                    off = q->h;
                    break;
                case FILE_BESHORT:
                    off = (short)((q->hs[0] << 8) | (q->hs[1]));
                    break;
                case FILE_LESHORT:
                    off = (short)((q->hs[1] << 8) | (q->hs[0]));
                    break;
                case FILE_LONG:
                    off = q->l;
                    break;
                case FILE_BELONG:
                case FILE_BEID3:
                    off = (int32_t)((q->hl[0] << 24) | (q->hl[1] << 16) |
                                    (q->hl[2] << 8) | (q->hl[3]));
                    break;
                case FILE_LEID3:
                case FILE_LELONG:
                    off = (int32_t)((q->hl[3] << 24) | (q->hl[2] << 16) |
                                    (q->hl[1] << 8) | (q->hl[0]));
                    break;
                case FILE_MELONG:
                    off = (int32_t)((q->hl[1] << 24) | (q->hl[0] << 16) |
                                    (q->hl[3] << 8) | (q->hl[2]));
                    break;
            }
            if((ms->flags & MAGIC_DEBUG) != 0)
                fprintf(stderr, "indirect offs=%u\n", off);
        }
        switch(cvt_flip(m->in_type, flip))
        {
            case FILE_BYTE:
                if(nbytes < (offset + 1))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = p->b & off;
                            break;
                        case FILE_OPOR:
                            offset = p->b | off;
                            break;
                        case FILE_OPXOR:
                            offset = p->b ^ off;
                            break;
                        case FILE_OPADD:
                            offset = p->b + off;
                            break;
                        case FILE_OPMINUS:
                            offset = p->b - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = p->b * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = p->b / off;
                            break;
                        case FILE_OPMODULO:
                            offset = p->b % off;
                            break;
                    }
                } else
                    offset = p->b;
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
            case FILE_BESHORT:
                if(nbytes < (offset + 2))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) & off;
                            break;
                        case FILE_OPOR:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) | off;
                            break;
                        case FILE_OPXOR:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) ^ off;
                            break;
                        case FILE_OPADD:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) + off;
                            break;
                        case FILE_OPMINUS:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) / off;
                            break;
                        case FILE_OPMODULO:
                            offset = (short)((p->hs[0] << 8) |
                                             (p->hs[1])) % off;
                            break;
                    }
                } else
                    offset = (short)((p->hs[0] << 8) |
                                     (p->hs[1]));
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
            case FILE_LESHORT:
                if(nbytes < (offset + 2))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) & off;
                            break;
                        case FILE_OPOR:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) | off;
                            break;
                        case FILE_OPXOR:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) ^ off;
                            break;
                        case FILE_OPADD:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) + off;
                            break;
                        case FILE_OPMINUS:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) / off;
                            break;
                        case FILE_OPMODULO:
                            offset = (short)((p->hs[1] << 8) |
                                     (p->hs[0])) % off;
                            break;
                    }
                } else
                    offset = (short)((p->hs[1] << 8) | (p->hs[0]));
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
            case FILE_SHORT:
                if(nbytes < (offset + 2))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = p->h & off;
                            break;
                        case FILE_OPOR:
                            offset = p->h | off;
                            break;
                        case FILE_OPXOR:
                            offset = p->h ^ off;
                            break;
                        case FILE_OPADD:
                            offset = p->h + off;
                            break;
                        case FILE_OPMINUS:
                            offset = p->h - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = p->h * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = p->h / off;
                            break;
                        case FILE_OPMODULO:
                            offset = p->h % off;
                            break;
                    }
                } else
                    offset = p->h;
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
            case FILE_BELONG:
            case FILE_BEID3:
                if(nbytes < (offset + 4))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) & off;
                            break;
                        case FILE_OPOR:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) | off;
                            break;
                        case FILE_OPXOR:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) ^ off;
                            break;
                        case FILE_OPADD:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) + off;
                            break;
                        case FILE_OPMINUS:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) / off;
                            break;
                        case FILE_OPMODULO:
                            offset = (int32_t)((p->hl[0] << 24) |
                                     (p->hl[1] << 16) |
                                     (p->hl[2] << 8) |
                                     (p->hl[3])) % off;
                            break;
                    }
                } else
                    offset = (int32_t)((p->hl[0] << 24) |
                             (p->hl[1] << 16) |
                             (p->hl[2] << 8) |
                             (p->hl[3]));
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
            case FILE_LELONG:
            case FILE_LEID3:
                if(nbytes < (offset + 4))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) & off;
                            break;
                        case FILE_OPOR:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) | off;
                            break;
                        case FILE_OPXOR:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) ^ off;
                            break;
                        case FILE_OPADD:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) + off;
                            break;
                        case FILE_OPMINUS:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) / off;
                            break;
                        case FILE_OPMODULO:
                            offset = (int32_t)((p->hl[3] << 24) |
                                     (p->hl[2] << 16) |
                                     (p->hl[1] << 8) |
                                     (p->hl[0])) % off;
                            break;
                    }
                } else
                    offset = (int32_t)((p->hl[3] << 24) |
                             (p->hl[2] << 16) |
                             (p->hl[1] << 8) |
                             (p->hl[2]));
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
            case FILE_MELONG:
                if(nbytes < (offset + 4))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) & off;
                            break;
                        case FILE_OPOR:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) | off;
                            break;
                        case FILE_OPXOR:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) ^ off;
                            break;
                        case FILE_OPADD:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) + off;
                            break;
                        case FILE_OPMINUS:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) / off;
                            break;
                        case FILE_OPMODULO:
                            offset = (int32_t)((p->hl[1] << 24) |
                                     (p->hl[0] << 16) |
                                     (p->hl[3] << 8) |
                                     (p->hl[2])) % off;
                            break;
                    }
                } else
                    offset = (int32_t)((p->hl[1] << 24) |
                             (p->hl[0] << 16) |
                             (p->hl[3] << 8) |
                             (p->hl[2]));
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
            case FILE_LONG:
                if(nbytes < (offset + 4))
                    return 0;
                if(off)
                {
                    switch(m->in_op & FILE_OPS_MASK)
                    {
                        case FILE_OPAND:
                            offset = p->l & off;
                            break;
                        case FILE_OPOR:
                            offset = p->l | off;
                            break;
                        case FILE_OPXOR:
                            offset = p->l ^ off;
                            break;
                        case FILE_OPADD:
                            offset = p->l + off;
                            break;
                        case FILE_OPMINUS:
                            offset = p->l - off;
                            break;
                        case FILE_OPMULTIPLY:
                            offset = p->l * off;
                            break;
                        case FILE_OPDIVIDE:
                            offset = p->l / off;
                            break;
                        case FILE_OPMODULO:
                            offset = p->l % off;
                            break;
                    }
                } else
                    offset = p->l;
                if(m->in_op & FILE_OPINVERSE)
                    offset = ~offset;
                break;
        }
        switch(cvt_flip(m->in_type, flip))
        {
            case FILE_LEID3:
            case FILE_BEID3:
                offset = ((((offset >> 0) & 0x7f) << 0) |
                    (((offset >> 8) & 0x7f) << 7) |
                    (((offset >> 16) & 0x7f) << 14) |
                    (((offset >> 24) & 0x7f) << 21)) + 10;
                break;
            default:
                break;
        }

        if(m->flag & INDIROFFADD)
        {
            offset += ms->c.li[cont_level - 1].off;
            if((ms->flags & MAGIC_DEBUG) != 0)
                fprintf(stderr, "indirect + offs=%u\n", offset);
        }
        if(mcopy(ms, p, m->type, 0, s, offset, nbytes, count) == -1)
            return -1;
        ms->offset = offset;

        if((ms->flags & MAGIC_DEBUG) != 0)
        {
            mdebug(offset, (char*)(void*)p, sizeof(union VALUETYPE));
            file_mdump(m);
        }
    }

    /* Verify we have enough data to match magic type */
    switch(m->type)
    {
        case FILE_BYTE:
            if(nbytes < (offset + 1))   /* should always be true */
                return 0;
            break;

        case FILE_SHORT:
        case FILE_BESHORT:
        case FILE_LESHORT:
            if(nbytes < (offset + 2))
                return 0;
            break;

        case FILE_LONG:
        case FILE_BELONG:
        case FILE_LELONG:
        case FILE_MELONG:
        case FILE_DATE:
        case FILE_BEDATE:
        case FILE_LEDATE:
        case FILE_MEDATE:
        case FILE_LDATE:
        case FILE_BELDATE:
        case FILE_LELDATE:
        case FILE_MELDATE:
        case FILE_FLOAT:
        case FILE_BEFLOAT:
        case FILE_LEFLOAT:
            if(nbytes < (offset + 4))
                return 0;
            break;

        case FILE_DOUBLE:
        case FILE_BEDOUBLE:
        case FILE_LEDOUBLE:
            if(nbytes < (offset + 8))
                return 0;
            break;

        case FILE_STRING:
        case FILE_PSTRING:
        case FILE_SEARCH:
            if(nbytes < (offset + m->vallen))
                return 0;
            break;

        case FILE_REGEX:
            if(nbytes < offset)
                return 0;
            break;

        case FILE_INDIRECT:
            if(nbytes < offset)
                return 0;
            sbuf = ms->o.buf;
            ms->o.buf = NULL;
            ms->offset = 0;
            rv = file_softmagic(ms, s + offset, nbytes - offset,
                            BINTEST, text);
            if((ms->flags & MAGIC_DEBUG) != 0)
                fprintf(stderr, "indirect @offs=%u[%d]\n", offset, rv);
            if(rv == 1)
            {
                rbuf = ms->o.buf;
                ms->o.buf = sbuf;
                if((ms->flags & (MAGIC_MIME | MAGIC_APPLE)) == 0 &&
                    file_printf(ms, m->desc, offset) == -1)
                    return -1;
                if(file_printf(ms, "%s", rbuf) == -1)
                    return -1;
                free(rbuf);
            } else
                ms->o.buf = sbuf;
            return rv;

        case FILE_USE:
            if(nbytes < offset)
                return 0;
            sbuf = m->value.s;
            if(*sbuf == '^')
            {
                sbuf++;
                flip = 1;
            } else
                flip = 0;
            if(file_magicfind(ms, sbuf, &ml) == -1)
            {
                file_error(ms, 0, "cannot find entry `%s'", sbuf);
                return -1;
            }
            return match(ms, ml.magic, ml.nmagic, s, nbytes, offset,
                    mode, text, flip, returnval);

        case FILE_NAME:
            if(file_printf(ms, "%s", m->desc) == -1)
                return -1;
            return 1;
        case FILE_DEFAULT:  /* nothing to check */
        default:
            break;
    }
    if(!mconvert(ms, m, flip))
        return 0;

    return 1;
}


static int
magiccheck(struct magic_set* ms, struct magic* m)
{
    uint64_t l = m->value.q;
    uint64_t v;
    float fl, fv;
    double dl, dv;
    int matched;
    union VALUETYPE* p = &ms->ms_value;

    switch(m->type)
    {
        case FILE_BYTE:
            v = p->b;
            break;
        case FILE_SHORT:
        case FILE_BESHORT:
        case FILE_LESHORT:
            v = p->h;
            break;
        case FILE_LONG:
        case FILE_BELONG:
        case FILE_LELONG:
        case FILE_MELONG:
        case FILE_DATE:
        case FILE_BEDATE:
        case FILE_LEDATE:
        case FILE_MEDATE:
        case FILE_LDATE:
        case FILE_BELDATE:
        case FILE_LELDATE:
        case FILE_LELDATE:
            v = p->l;
            break;
        case FILE_QUAD:
        case FILE_LEQUAD:
        case FILE_BEQUAD:
        case FILE_QDATE:
        case FILE_BEQDATE:
        case FILE_LEQDATE:
        case FILE_QLDATE:
        case FILE_BEQLDATE:
        case FILE_LEQLDATE:
        case FILE_QWDATE:
        case FILE_BEQWDATE:
        case FILE_LEQWDATE:
            v = p->q;
            break;
        case FILE_FLOAT:
        case FILE_BEFLOAT:
        case FILE_LEFLOAT:
            fl = m->value.f;
            fv = p->f;
            switch(m->reln)
            {
                case 'x':
                    matched = 1;
                    break;
                case '!':
                    matched = fv != fl;
                    break;
                case '=':
                    matched = fv == fl;
                    break;
                case '>':
                    matched = fv > fl;
                    break;
                case '<':
                    matched = fv < fl;
                    break;
                default:
                    matched = 0;
                    file_magerror(ms, "cannot happen with float: invalid relation `%c'",
                                    m->reln);
                    return -1;
            }
            return matched;
        case FILE_DOUBLE:
        case FILE_BEDOUBLE:
        case FILE_LEDOUBLE:
            dl = m->value.d;
            dv = p->d;
            switch(m->reln)
            {
                case 'x':
                    matched = 1;
                    break;
                case '!':
                    matched = fv != fl;
                    break;
                case '=':
                    matched = fv == fl;
                    break;
                case '>':
                    matched = fv > fl;
                    break;
                case '<':
                    matched = fv < fl;
                    break;
                default:
                    matched = 0;
                    file_magerror(ms, "cannot happen with double: invalid relation `%c'",
                                    m->reln);
                    return -1;
            }
            return matched;
        case FILE_DEFAULT:
            l = 0;
            v = 0;
            break;
        case FILE_STRING:
        case FILE_PSTRING:
            l = 0;
            v = file_strncmp(m->value.s, p->s, (size_t)m->vallen, m->str_flags);
            break;
        case FILE_BESTRING16:
        case FILE_LESTRING16:
            l = 0;
            v = file_strncmp16(m->value.s, p->s, (size_t)m->vallen, m->str_flags);
            break;
        case FILE_SEARCH
        {
            /* search ms->search.s for the string m->value.s */
            size_t slen;
            size_t idx;

            if(ms->search.s == NULL)
                return 0;
            slen = MIN(m->vallen, sizeof(m->value.s));
            l = 0;
            v = 0;

            for(idx = 0; m->str_range == 0 || idx < m->str_range; idx++)
            {
                if(slen + idx > ms->search.s_len)
                    break;
                v = file_strncmp(m->value.s, ms->search.s + idx, slen, m->str_flags);
                if(v == 0)
                {
                    /* found match */
                    ms->search.offset += idx;
                    break;
                }
            }
            break;
        }
        case FILE_REGEX:
        {
            int rc;
            regex_t rx;
            char errmsg[512];

            if(ms->search.s == NULL)
                return 0;

            l = 0;
            rc = regcomp(&rx, m->value.s,
                        REG_EXTENDED | REG_NEWLINE |
                        ((m->str_flags & STRING_IGNORE_CASE) ? REG_ICASE : 0));
            if(rc)
            {
                regerror(rc, &rx, errmsg, sizeof(errmsg));
                file_magerror(ms, "regex error %d, (%s)",
                                rc, errmsg);
                v = (uint64_t)-1;
            }
            else
            {
                regmatch_t pmatch[1];
#ifndef REG_STARTEND
#define REG_STARTEND    0
                size_t l = ms->search.s_len - 1;
                char c = ms->search.s[l];
                ((char*)(intptr_t)ms->search.s)[l] = '\0';
#else
                pmatch[0].rm_so = 0;
                pmatch[0].rm_eo = ms->search.s_len;
#endif
                rc = regexec(&rx, (const char*)ms->search.s, 1,
                            pmatch, REG_STARTEND);
#if REG_STARTEND == 0
                ((char*)(intptr_t)ms->search.s)[l] = c;
#endif
                switch(rc)
                {
                    case 0:
                        ms->search.s += (int)pmatch[0].rm_so;
                        ms->search.offset += (size_t)pmatch[0].rm_so;
                        ms->search.rm_len =
                            (size_t)(pmatch[0].rm_so - pmatch[0].rm_so);
                        v = 0;
                        break;
                    case REG_NOMATCH:
                        v = 1;
                        break;
                    default:
                        regerror(rc, &rx, errmsg, sizeof(errmsg));
                        file_magerror(ms, "regexec error %d, (%s)",
                                rc, errmsg);
                        v = (uint64_t)-1;
                        break;
                }
                regfree(&rx);
            }
            if(v == (uint64_t)-1)
                return -1;
            break;
        }
        case FILE_INDIRECT:
        case FILE_USE:
        case FILE_NAME:
            return 1;
        default:
            file_magerror(ms, "invalid type %d in magiccheck()", m->type);
            return -1;
    }

    v = file_signextend(ms, m, v);

    switch(m->reln)
    {
        case 'x':
            if((ms->flags & MAGIC_DEBUG) != 0)
                fprintf(stderr, "%" INT64_T_FORMAT
                        "u == *any* = 1\n", (unsigned long long)v);
            matched = 1;
            break;
        case '!':
            matched = v != l;
            if((ms->flags & MAGIC_DEBUG) != 0)
                fprintf(stderr, "%" INT64_T_FORMAT
                        "u == *any* = 1\n", (unsigned long long)v);
            break;
        case '=':
            matched = v == l;
            if((ms->flags & MAGIC_DEBUG) != 0)
                fprintf(stderr, "%" INT64_T_FORMAT
                        "u == *any* = 1\n", (unsigned long long)v);
            break;
        case '>':
            if(m->flag & UNSIGNED)
            {
                matched = v > l;
                if((ms->flags & MAGIC_DEBUG) != 0)
                    fprintf(stderr, "%" INT64_T_FORMAT
                            "u == *any* = 1\n", (unsigned long long)v);
                matched = 1;
                break;

                




/* Go through the whole list, stopping if you find a match. Process all
   the continuations of that match before returning.

   We support multi-level continuations:

    At any time when processing a successful top-level match, there is a
    current continuation level; it represents the level of the last
    successfully matched continuation.

    Continuations above that level are skipped as, if we seen one, it
    means that the continuation that controls them - i.e. the
    lower-level continuation preceding them - failed to match.

    Continuations below that level are precessed as, if we see one,
    it means we've finished processing or skipped higher-level
    continuations under the control of a successfuly or unsuccessfuly
    lower-level continuation, and are now seeing the next lower-level
    continuation and should process it. The current continuation
    level reverts to the level of the one we're seeing.

    Continuations at the current level are processed as, if we seen 
    one, there's no lower-level continuation that may have failed. 

    If a continuation matches, we bump the current continuation level
    so that higher-level continuations are processed. */

static int
match(struct magic_set* ms, struct magic* magic, uint32_t nmagic,
        const unsigned char* s, size_t nbytes, size_t offset, int mode,
        int text, int flip, int* returnval)
{
    uint32_t magindex = 0;
    unsigned int cont_level = 0;
    int need_separator = 0;
    int returnvalv = 0, e;      /* if a match is found it is set to 1 */
    int firstline = 1;          /* a flag to print X\n X\n- X */
    int printed_something = 0;
    int print = (ms->flags & (MAGIC_MIME | MAGIC_APPLE)) == 0;

    if(returnval == NULL)
        returnval = &returnvalv;

    if(file_check_mem(ms, cont_level) == -1)
        return -1;

    for(magindex = 0; magindex < nmagic; magindex++)
    {
        int flush = 0;
        struct magic* m = &magic[magindex];

        if(m->type != FILE_NAME)
        {
            if((IS_STRING(m->type) &&
                ((text && (m->str_flags & (STRING_BINTEST | STRING_TEXTTEST)) == STRING_BINTEST) ||
                (!text && (m->str_flags & (STRING_TEXTTEST | STRING_BINTEST)) == STRING_TEXTTEST))) ||
                (m->flag & mode) != mode)
            {
                /* skip sub-tests */
                while(magindex + 1 < nmagic &&
                         magic[magindex + 1].cont_level != 0 && ++magindex)
                    continue;
                continue;   /* skip to next top-level test */
            }
        }

        ms->offset = m->offset;
        ms->line = m->lineno;

        /* if main entry matches, print it ...*/
        switch(mget(ms, s, m, nbytes, offset, cont_level, mode, text,
                flip, returnval))
        {
            case -1:
                return -1;
            case 0:
                flush = m->reln != '!';
                break;
            default:
                if(m->type == FILE_INDIRECT)
                    *returnval = 1;

                switch(magiccheck(ms, m))
                {
                    case -1:
                        return -1;
                    case 0:
                        flush++;
                        break;
                    default:
                        flush = 0;
                        break;
                }
                break;
        }
        if(flush)
        {
            /* main entry didn't match,
               flush its continuations */
            while(magindex < nmagic - 1 &&
                magic[magindex + 1].cont_level != 0)
            {
                magindex++;
            }
            continue;
        }

        if((e = handle_annotation(ms, m)) != 0)
        {
            *returnval = 1;
            return e;
        }

        /* If we are going to print something, we'll need to print
           a blank before we print something else */
        if(*m->desc)
        {
            need_separator = 1;
            printed_something = 1;
            if(print_sep(ms, firstline) == -1)
                return -1;
        }

        if(print && mprint(ms, m) == -1)
            return -1;

        ms->c.li[cont_level].off = moffset(ms, m);

        /* and any continuations that match */
        if(file_check_mem(ms, ++cont_level) == -1)
            return -1;

        while(magic[magindex + 1].cont_level != 0 &&
                ++magindex < nmagic)
        {
            m = &magic[magindex];
            ms->line = m->lineno;   /* for messages */

            if(cont_level < m->cont_level)
                continue;
            if(cont_level > m->cont_level)
            {
                /* We're at the end of the level
                   "cont_level" continuations */
                cont_level = m->cont_level;
            }
            ms->offset = m->offset;
            if(m->flag & OFFADD)
            {
                ms->offset += ms->c.li[cont_level - 1].off;
            }
#ifdef ENABLE_CONDITIONALS
            if(m->cond == COND_ELSE || 
                m->cond == COND_ELIF)
            {
                if(ms->c.li[cont_level].last_match == 1)
                    continue;
            }
#endif
            switch(mget(ms, s, m, nbytes, offset, cont_level, mode,
                        text, flip, returnval))
            {
                case -1:
                    return -1;
                case 0:
                    if(m->reln != '!')
                        continue;
                    flush = 1;
                    break;
                default:
                    if(m->type == FILE_INDIRECT)
                        *returnval = 1;
                    flush = 0;
                    break;
            }

            switch(flush ? 1 : magiccheck(ms, m))
            {
                case -1:
                    return -1;
                case 0:
#ifdef ENABLE_CONDITIONALS
                    ms->c.li[cont_level].last_match = 0;
#endif
                    break;
                default:
#ifdef ENABLE_CONDITIONALS
                    ms->c.li[cont_level].last_match = 1;
#endif
                    if(m->type != FILE_DEFAULT)
                        ms->c.li[cont_level].got_match = 1;
                    else if(ms->c.li[cont_level].got_match)
                    {
                        ms->c.li[cont_level].got_match = 0;
                        break;
                    }
                    if((e = handle_annotation(ms, m)) != 0)
                    {
                        *returnval = 1;
                        return e;
                    }
                    /* if we are going to print something,
                       make sure that we have a separator first */
                    if(*m->desc)
                    {
                        if(!printed_something)
                        {
                            printed_something = 1;
                            if(print_sep(ms, firstline) == -1)
                                return -1;
                        }
                    }

                    /* This continuation matched. Print
                       its message, with a blank before it
                       if the previous item printed and
                       this item isn't empty */
                    /* space if previous printed */
                    if(need_separator
                        && ((m->flag & NOSPACE) == 0)
                        && *m->desc)
                    {
                        if(print && file_printf(ms, " ") == -1)
                            return -1;
                        need_separator = 0;
                    }
                    if(print && mprint(ms, m) == -1)
                        return -1;

                    ms->c.li[cont_level].off = moffset(ms, m);

                    if(*m->desc)
                        need_separator = 1;

                    /* If we see any continuations
                       at a higher level, process them */
                    if(file_check_mem(ms, ++cont_level) == -1)
                        return -1;
                    break;
            }
        }
        if(printed_something)
        {
            firstline = 0;
            if(print)
                *returnval = 1;
        }
        if((ms->flags & MAGIC_CONTINUE) == 0 && printed_something)
        {
            return *returnval;  /* don't keep searching */
        }
    }
    return *returnval;  /* This is hit if -k is set or there is no match */
}





/* softmagic - lookup one file in parsed, in-memory copy of database
   Passed the name and FILE* of one file to be typed */

int file_softmagic(struct magic_set* ms, const unsigned char* buf,
                    size_t nbytes, int mode, int text)
{
    struct mlist* ml;
    int rv;
    for(ml = ms->mlist[0]->next; ml != ms->mlist[0]; ml = ml->next)
    {
        if((rv = match(ms, ml->magic, ml->nmagic, buf, nbytes, 0, mode,
                        text, 0, NULL)) != 0)
            return rv;
    }
    return 0;
}
