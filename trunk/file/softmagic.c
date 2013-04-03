/* softmagic - interpret variable magic from MAGIC */

#include "file_.h"
#include "magic_.h"


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
                    return;
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
        if(mcopy(ms, p->type, 0, s, offset, nbyte, count) == -1)
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
